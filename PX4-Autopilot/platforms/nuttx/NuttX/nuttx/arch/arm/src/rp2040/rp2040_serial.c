/****************************************************************************
 * arch/arm/src/rp2040/rp2040_serial.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <debug.h>

#ifdef CONFIG_SERIAL_TERMIOS
#  include <termios.h>
#endif

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/fs/ioctl.h>
#include <nuttx/serial/serial.h>
#include <nuttx/spinlock.h>

#include <arch/board/board.h>

#include "chip.h"
#include "arm_internal.h"
#include "rp2040_config.h"
#include "rp2040_serial.h"

/****************************************************************************
 * Pre-processor definitions
 ****************************************************************************/

/* If we are not using the serial driver for the console, then we still must
 * provide some minimal implementation of up_putc.
 */

#if defined(USE_SERIALDRIVER) && defined(HAVE_UART)

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct up_dev_s
{
  uintptr_t uartbase; /* Base address of UART registers */
  uint32_t basefreq;  /* Base frequency of input clock */
  uint32_t baud;      /* Configured baud */
  uint32_t ier;       /* Saved IER value */
  uint8_t id;         /* ID=0,1,2,3 */
  uint8_t irq;        /* IRQ associated with this UART */
  uint8_t parity;     /* 0=none, 1=odd, 2=even */
  uint8_t bits;       /* Number of bits (5,6,7 or 8) */
  bool stopbits2;     /* true: Configure with 2 stop bits instead of 1 */
#ifdef CONFIG_SERIAL_IFLOWCONTROL
  bool iflow;         /* input flow control (RTS) enabled */
#endif
#ifdef CONFIG_SERIAL_OFLOWCONTROL
  bool oflow;         /* output flow control (CTS) enabled */
#endif
  spinlock_t lock;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

#ifndef CONFIG_SUPPRESS_UART_CONFIG
static void up_set_format(struct uart_dev_s *dev);
#endif
static int up_setup(struct uart_dev_s *dev);
static void up_shutdown(struct uart_dev_s *dev);
static int up_attach(struct uart_dev_s *dev);
static void up_detach(struct uart_dev_s *dev);
static int up_interrupt(int irq, void *context, void *arg);
static int up_ioctl(struct file *filep, int cmd, unsigned long arg);
#ifdef CONFIG_SERIAL_IFLOWCONTROL
static bool up_rxflowcontrol(struct uart_dev_s *dev,
                             unsigned int nbuffered, bool upper);
#endif
static int up_receive(struct uart_dev_s *dev, unsigned int *status);
static void up_rxint(struct uart_dev_s *dev, bool enable);
static bool up_rxavailable(struct uart_dev_s *dev);
static void up_send(struct uart_dev_s *dev, int ch);
static void up_txint(struct uart_dev_s *dev, bool enable);
static bool up_txready(struct uart_dev_s *dev);
static bool up_txempty(struct uart_dev_s *dev);
static inline uint32_t up_serialin(struct up_dev_s *priv, int offset);

#ifdef CONFIG_ARCH_CHIP_RP2040
bool rp2040_uart_tryreceive(FAR struct uart_dev_s *dev, FAR char *ch);
bool rp2040_uart_diag_repair(FAR struct uart_dev_s *dev, char src);

static inline void rp2040_diag_hex32(uintptr_t v)
{
  int i;

  for (i = 28; i >= 0; i -= 4)
    {
      uint32_t nibble = (v >> i) & 0x0f;
      arm_lowputc((char)(nibble < 10 ? ('0' + nibble) : ('A' + nibble - 10)));
    }
}

static inline void rp2040_diag_dump_words(FAR const void *ptr, int nwords)
{
  FAR const uint32_t *p = (FAR const uint32_t *)ptr;
  int i;

  for (i = 0; i < nwords; i++)
    {
      if (i > 0)
        {
          arm_lowputc('.');
        }

      rp2040_diag_hex32((uintptr_t)p[i]);
    }
}
#endif
static const struct uart_ops_s g_uart_ops;

/****************************************************************************
 * Private Data
 ****************************************************************************/


#ifdef CONFIG_ARCH_CHIP_RP2040
bool rp2040_uart_tryreceive(FAR struct uart_dev_s *dev, FAR char *ch)
{
  FAR struct up_dev_s *priv;
  uint32_t fr;
  uint32_t dr;

  if (dev == NULL || ch == NULL || dev->priv == NULL)
    {
      return false;
    }

  /* Reject non-RP2040 UART devices routed through generic serial.c. */

  if (dev->ops != &g_uart_ops)
    {
      return false;
    }

  priv = (FAR struct up_dev_s *)dev->priv;
  fr = up_serialin(priv, RP2040_UART_UARTFR_OFFSET);

  if ((fr & RP2040_UART_UARTFR_RXFE) != 0)
    {
      return false;
    }

  dr = up_serialin(priv, RP2040_UART_UARTDR_OFFSET);
  *ch = (char)(dr & 0xff);
  return true;
}
#endif
static const struct uart_ops_s g_uart_ops =
{
  .setup         = up_setup,
  .shutdown      = up_shutdown,
  .attach        = up_attach,
  .detach        = up_detach,
  .ioctl         = up_ioctl,
  .receive       = up_receive,
  .rxint         = up_rxint,
  .rxavailable   = up_rxavailable,
#ifdef CONFIG_SERIAL_IFLOWCONTROL
  .rxflowcontrol = up_rxflowcontrol,
#endif
  .send          = up_send,
  .txint         = up_txint,
  .txready       = up_txready,
  .txempty       = up_txempty,
};

/* I/O buffers */

#ifdef CONFIG_ARCH_CHIP_RP2040
#define RP2040_UART_CANARY_PRE  0x13579bdfu
#define RP2040_UART_CANARY_POST 0x2468ace0u

struct rp2040_guarded_buf0_s
{
  uint32_t pre;
  char data[CONFIG_UART0_RXBUFSIZE];
  uint32_t post;
};

#ifdef CONFIG_RP2040_UART1
struct rp2040_guarded_buf1_s
{
  uint32_t pre;
  char data[CONFIG_UART1_RXBUFSIZE];
  uint32_t post;
};
#endif
#endif

#ifdef CONFIG_RP2040_UART0
#ifdef CONFIG_ARCH_CHIP_RP2040
static struct rp2040_guarded_buf0_s g_uart0rxguard =
{
  .pre  = RP2040_UART_CANARY_PRE,
  .post = RP2040_UART_CANARY_POST,
};
static uint32_t g_uart0port_pre = RP2040_UART_CANARY_PRE;
static uint32_t g_uart0port_post = RP2040_UART_CANARY_POST;
#else
static char g_uart0rxbuffer[CONFIG_UART0_RXBUFSIZE];
#endif
static char g_uart0txbuffer[CONFIG_UART0_TXBUFSIZE];
#endif
#ifdef CONFIG_RP2040_UART1
#ifdef CONFIG_ARCH_CHIP_RP2040
static struct rp2040_guarded_buf1_s g_uart1rxguard =
{
  .pre  = RP2040_UART_CANARY_PRE,
  .post = RP2040_UART_CANARY_POST,
};
static uint32_t g_uart1port_pre = RP2040_UART_CANARY_PRE;
static uint32_t g_uart1port_post = RP2040_UART_CANARY_POST;
#else
static char g_uart1rxbuffer[CONFIG_UART1_RXBUFSIZE];
#endif
static char g_uart1txbuffer[CONFIG_UART1_TXBUFSIZE];
#endif

/* This describes the state of the RP2040 UART0 port. */

#ifdef CONFIG_RP2040_UART0
#ifdef CONFIG_ARCH_CHIP_RP2040
static uint32_t g_uart0priv_pre = RP2040_UART_CANARY_PRE;
#endif
static struct up_dev_s g_uart0priv =
{
  .uartbase  = RP2040_UART0_BASE,
  .basefreq  = BOARD_UART_BASEFREQ,
  .baud      = CONFIG_UART0_BAUD,
  .id        = 0,
  .irq       = RP2040_UART0_IRQ,
  .parity    = CONFIG_UART0_PARITY,
  .bits      = CONFIG_UART0_BITS,
  .stopbits2 = CONFIG_UART0_2STOP,
#if defined(CONFIG_SERIAL_IFLOWCONTROL) && defined(CONFIG_UART0_IFLOWCONTROL)
  .iflow     = true,
#endif
#if defined(CONFIG_SERIAL_IFLOWCONTROL) && defined(CONFIG_UART0_OFLOWCONTROL)
  .oflow     = true,
#endif
};
#ifdef CONFIG_ARCH_CHIP_RP2040
static uint32_t g_uart0priv_post = RP2040_UART_CANARY_POST;
static uint32_t g_uart0port_pre2 = RP2040_UART_CANARY_PRE;
#endif

static uart_dev_t g_uart0port =
{
  .recv =
    {
      .size   = CONFIG_UART0_RXBUFSIZE,
#ifdef CONFIG_ARCH_CHIP_RP2040
      .buffer = g_uart0rxguard.data,
#else
      .buffer = g_uart0rxbuffer,
#endif
    },
  .xmit =
    {
      .size   = CONFIG_UART0_TXBUFSIZE,
      .buffer = g_uart0txbuffer,
    },
  .ops  = &g_uart_ops,
  .priv = &g_uart0priv,
};
#ifdef CONFIG_ARCH_CHIP_RP2040
static uint32_t g_uart0port_post2 = RP2040_UART_CANARY_POST;
#endif
#  define TTYS0_DEV g_uart0port /* UART0=ttyS0 */
#endif

/* This describes the state of the RP2040 UART1 port. */

#ifdef CONFIG_RP2040_UART1
#ifdef CONFIG_ARCH_CHIP_RP2040
static uint32_t g_uart1priv_pre = RP2040_UART_CANARY_PRE;
#endif
static struct up_dev_s g_uart1priv =
{
  .uartbase  = RP2040_UART1_BASE,
  .basefreq  = BOARD_UART_BASEFREQ,
  .baud      = CONFIG_UART1_BAUD,
  .id        = 1,
  .irq       = RP2040_UART1_IRQ,
  .parity    = CONFIG_UART1_PARITY,
  .bits      = CONFIG_UART1_BITS,
  .stopbits2 = CONFIG_UART1_2STOP,
#if defined(CONFIG_SERIAL_IFLOWCONTROL) && defined(CONFIG_UART1_IFLOWCONTROL)
  .iflow     = true,
#endif
#if defined(CONFIG_SERIAL_IFLOWCONTROL) && defined(CONFIG_UART1_OFLOWCONTROL)
  .oflow     = true,
#endif
};
#ifdef CONFIG_ARCH_CHIP_RP2040
static uint32_t g_uart1priv_post = RP2040_UART_CANARY_POST;
static uint32_t g_uart1port_pre2 = RP2040_UART_CANARY_PRE;
#endif

static uart_dev_t g_uart1port =
{
  .recv =
    {
      .size   = CONFIG_UART1_RXBUFSIZE,
#ifdef CONFIG_ARCH_CHIP_RP2040
      .buffer = g_uart1rxguard.data,
#else
      .buffer = g_uart1rxbuffer,
#endif
    },
  .xmit =
    {
      .size   = CONFIG_UART1_TXBUFSIZE,
      .buffer = g_uart1txbuffer,
    },
  .ops  = &g_uart_ops,
  .priv = &g_uart1priv,
};
#ifdef CONFIG_ARCH_CHIP_RP2040
static uint32_t g_uart1port_post2 = RP2040_UART_CANARY_POST;
#endif
#  define TTYS1_DEV g_uart1port /* UART1=ttyS1 */
#endif

/* Which UART with be tty0/console and which tty1? tty2? tty3? */

#ifdef HAVE_CONSOLE
#  if defined(CONFIG_UART0_SERIAL_CONSOLE)
#    define CONSOLE_DEV     g_uart0port /* UART0=console */
#  elif defined(CONFIG_UART1_SERIAL_CONSOLE)
#    define CONSOLE_DEV     g_uart1port /* UART1=console */
#  endif
#endif /* HAVE_CONSOLE */

#ifdef CONFIG_ARCH_CHIP_RP2040
bool rp2040_uart_diag_repair(FAR struct uart_dev_s *dev, char src)
{
  bool ok = true;

#ifdef CONFIG_RP2040_UART0
  static uint16_t uart0_diag_bits;
  bool repaired = false;

  if (dev == &g_uart0port)
    {
      if (dev->recv.size == 0)
        {
          ok = false;
          dev->recv.size = CONFIG_UART0_RXBUFSIZE;
        }

      if (dev->ops != &g_uart_ops)
        {
          uintptr_t bad = (uintptr_t)dev->ops;
          ok = false;
          repaired = true;
          dev->ops = &g_uart_ops;
          if ((uart0_diag_bits & (1u << 5)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('O');
              arm_lowputc('[');
              rp2040_diag_hex32(bad);
              arm_lowputc(']');
              uart0_diag_bits |= (1u << 5);
            }
        }

      if (dev->priv != &g_uart0priv)
        {
          uintptr_t bad = (uintptr_t)dev->priv;
          ok = false;
          repaired = true;
          dev->priv = &g_uart0priv;
          if ((uart0_diag_bits & (1u << 6)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('V');
              arm_lowputc('[');
              rp2040_diag_hex32(bad);
              arm_lowputc(']');
              uart0_diag_bits |= (1u << 6);
            }
        }

      if (g_uart0priv.uartbase != RP2040_UART0_BASE ||
          g_uart0priv.basefreq != BOARD_UART_BASEFREQ ||
          g_uart0priv.id != 0 ||
          g_uart0priv.irq != RP2040_UART0_IRQ ||
          g_uart0priv.parity != CONFIG_UART0_PARITY ||
          g_uart0priv.bits != CONFIG_UART0_BITS ||
          g_uart0priv.stopbits2 != CONFIG_UART0_2STOP)
        {
          ok = false;
          repaired = true;
          g_uart0priv.uartbase = RP2040_UART0_BASE;
          g_uart0priv.basefreq = BOARD_UART_BASEFREQ;
          g_uart0priv.id = 0;
          g_uart0priv.irq = RP2040_UART0_IRQ;
          g_uart0priv.parity = CONFIG_UART0_PARITY;
          g_uart0priv.bits = CONFIG_UART0_BITS;
          g_uart0priv.stopbits2 = CONFIG_UART0_2STOP;
          if ((uart0_diag_bits & (1u << 11)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('B');
              uart0_diag_bits |= (1u << 11);
            }
        }

      if (dev->xmit.buffer != g_uart0txbuffer)
        {
          uintptr_t bad = (uintptr_t)dev->xmit.buffer;
          ok = false;
          repaired = true;
          dev->xmit.buffer = g_uart0txbuffer;
          if ((uart0_diag_bits & (1u << 7)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('Q');
              arm_lowputc('[');
              rp2040_diag_hex32(bad);
              arm_lowputc(']');
              uart0_diag_bits |= (1u << 7);
            }
        }

      if (dev->xmit.size != CONFIG_UART0_TXBUFSIZE)
        {
          ok = false;
          repaired = true;
          dev->xmit.size = CONFIG_UART0_TXBUFSIZE;
          if ((uart0_diag_bits & (1u << 8)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('q');
              uart0_diag_bits |= (1u << 8);
            }
        }

      if (g_uart0rxguard.pre != RP2040_UART_CANARY_PRE ||
          g_uart0rxguard.post != RP2040_UART_CANARY_POST ||
          g_uart0port_pre != RP2040_UART_CANARY_PRE ||
          g_uart0port_post != RP2040_UART_CANARY_POST ||
          g_uart0priv_pre != RP2040_UART_CANARY_PRE ||
          g_uart0priv_post != RP2040_UART_CANARY_POST ||
          g_uart0port_pre2 != RP2040_UART_CANARY_PRE ||
          g_uart0port_post2 != RP2040_UART_CANARY_POST)
        {
          ok = false;
          repaired = true;
          g_uart0rxguard.pre = RP2040_UART_CANARY_PRE;
          g_uart0rxguard.post = RP2040_UART_CANARY_POST;
          g_uart0port_pre = RP2040_UART_CANARY_PRE;
          g_uart0port_post = RP2040_UART_CANARY_POST;
          g_uart0priv_pre = RP2040_UART_CANARY_PRE;
          g_uart0priv_post = RP2040_UART_CANARY_POST;
          g_uart0port_pre2 = RP2040_UART_CANARY_PRE;
          g_uart0port_post2 = RP2040_UART_CANARY_POST;
          if ((uart0_diag_bits & (1u << 0)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('C');
              uart0_diag_bits |= (1u << 0);
            }
        }

      if (dev->recv.buffer != g_uart0rxguard.data)
        {
          uintptr_t bad = (uintptr_t)dev->recv.buffer;
          ok = false;
          repaired = true;
          dev->recv.buffer = g_uart0rxguard.data;
          if ((uart0_diag_bits & (1u << 1)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('P');
              arm_lowputc('[');
              rp2040_diag_hex32(bad);
              arm_lowputc(']');
              uart0_diag_bits |= (1u << 1);
            }
        }

      if (dev->recv.size != CONFIG_UART0_RXBUFSIZE)
        {
          ok = false;
          repaired = true;
          dev->recv.size = CONFIG_UART0_RXBUFSIZE;
          if ((uart0_diag_bits & (1u << 2)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('S');
              uart0_diag_bits |= (1u << 2);
            }
        }

      if (dev->recv.head >= dev->recv.size)
        {
          ok = false;
          repaired = true;
          dev->recv.head = 0;
          if ((uart0_diag_bits & (1u << 3)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('H');
              uart0_diag_bits |= (1u << 3);
            }
        }

      if (dev->recv.tail >= dev->recv.size)
        {
          ok = false;
          repaired = true;
          dev->recv.tail = 0;
          if ((uart0_diag_bits & (1u << 4)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('T');
              uart0_diag_bits |= (1u << 4);
            }
        }

      if (dev->xmit.head >= dev->xmit.size)
        {
          ok = false;
          repaired = true;
          dev->xmit.head = 0;
          if ((uart0_diag_bits & (1u << 9)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('U');
              uart0_diag_bits |= (1u << 9);
            }
        }

      if (dev->xmit.tail >= dev->xmit.size)
        {
          ok = false;
          repaired = true;
          dev->xmit.tail = 0;
          if ((uart0_diag_bits & (1u << 10)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('u');
              uart0_diag_bits |= (1u << 10);
            }
        }

      if (repaired)
        {
          if ((uart0_diag_bits & (1u << 12)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('D');
              arm_lowputc('{');
              rp2040_diag_dump_words(dev, 12);
              arm_lowputc('}');
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('E');
              arm_lowputc('{');
              rp2040_diag_dump_words(&g_uart0priv, 6);
              arm_lowputc('}');
              uart0_diag_bits |= (1u << 12);
            }

          /* Hard re-seed critical invariants for UART0 descriptor without
           * touching semaphore state in recv/xmit buffers.
           */

          dev->ops = &g_uart_ops;
          dev->priv = &g_uart0priv;
          dev->recv.buffer = g_uart0rxguard.data;
          dev->recv.size = CONFIG_UART0_RXBUFSIZE;
          if (dev->recv.head >= dev->recv.size)
            {
              dev->recv.head = 0;
            }

          if (dev->recv.tail >= dev->recv.size)
            {
              dev->recv.tail = 0;
            }

          dev->xmit.buffer = g_uart0txbuffer;
          dev->xmit.size = CONFIG_UART0_TXBUFSIZE;
          if (dev->xmit.head >= dev->xmit.size)
            {
              dev->xmit.head = 0;
            }

          if (dev->xmit.tail >= dev->xmit.size)
            {
              dev->xmit.tail = 0;
            }
        }

      return ok;
    }
#endif

#ifdef CONFIG_RP2040_UART1
  static uint16_t uart1_diag_bits;
  bool repaired1 = false;

  if (dev == &g_uart1port)
    {
      if (dev->recv.size == 0)
        {
          ok = false;
          dev->recv.size = CONFIG_UART1_RXBUFSIZE;
        }

      if (dev->ops != &g_uart_ops)
        {
          ok = false;
          repaired1 = true;
          dev->ops = &g_uart_ops;
          if ((uart1_diag_bits & (1u << 5)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('o');
              uart1_diag_bits |= (1u << 5);
            }
        }

      if (dev->priv != &g_uart1priv)
        {
          ok = false;
          repaired1 = true;
          dev->priv = &g_uart1priv;
          if ((uart1_diag_bits & (1u << 6)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('v');
              uart1_diag_bits |= (1u << 6);
            }
        }

      if (g_uart1priv.uartbase != RP2040_UART1_BASE ||
          g_uart1priv.basefreq != BOARD_UART_BASEFREQ ||
          g_uart1priv.id != 1 ||
          g_uart1priv.irq != RP2040_UART1_IRQ ||
          g_uart1priv.parity != CONFIG_UART1_PARITY ||
          g_uart1priv.bits != CONFIG_UART1_BITS ||
          g_uart1priv.stopbits2 != CONFIG_UART1_2STOP)
        {
          ok = false;
          repaired1 = true;
          g_uart1priv.uartbase = RP2040_UART1_BASE;
          g_uart1priv.basefreq = BOARD_UART_BASEFREQ;
          g_uart1priv.id = 1;
          g_uart1priv.irq = RP2040_UART1_IRQ;
          g_uart1priv.parity = CONFIG_UART1_PARITY;
          g_uart1priv.bits = CONFIG_UART1_BITS;
          g_uart1priv.stopbits2 = CONFIG_UART1_2STOP;
          if ((uart1_diag_bits & (1u << 11)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('b');
              uart1_diag_bits |= (1u << 11);
            }
        }

      if (dev->xmit.buffer != g_uart1txbuffer)
        {
          ok = false;
          repaired1 = true;
          dev->xmit.buffer = g_uart1txbuffer;
          if ((uart1_diag_bits & (1u << 7)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('w');
              uart1_diag_bits |= (1u << 7);
            }
        }

      if (dev->xmit.size != CONFIG_UART1_TXBUFSIZE)
        {
          ok = false;
          repaired1 = true;
          dev->xmit.size = CONFIG_UART1_TXBUFSIZE;
          if ((uart1_diag_bits & (1u << 8)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('W');
              uart1_diag_bits |= (1u << 8);
            }
        }

      if (g_uart1rxguard.pre != RP2040_UART_CANARY_PRE ||
          g_uart1rxguard.post != RP2040_UART_CANARY_POST ||
          g_uart1port_pre != RP2040_UART_CANARY_PRE ||
          g_uart1port_post != RP2040_UART_CANARY_POST ||
          g_uart1priv_pre != RP2040_UART_CANARY_PRE ||
          g_uart1priv_post != RP2040_UART_CANARY_POST ||
          g_uart1port_pre2 != RP2040_UART_CANARY_PRE ||
          g_uart1port_post2 != RP2040_UART_CANARY_POST)
        {
          ok = false;
          repaired1 = true;
          g_uart1rxguard.pre = RP2040_UART_CANARY_PRE;
          g_uart1rxguard.post = RP2040_UART_CANARY_POST;
          g_uart1port_pre = RP2040_UART_CANARY_PRE;
          g_uart1port_post = RP2040_UART_CANARY_POST;
          g_uart1priv_pre = RP2040_UART_CANARY_PRE;
          g_uart1priv_post = RP2040_UART_CANARY_POST;
          g_uart1port_pre2 = RP2040_UART_CANARY_PRE;
          g_uart1port_post2 = RP2040_UART_CANARY_POST;
          if ((uart1_diag_bits & (1u << 0)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('c');
              uart1_diag_bits |= (1u << 0);
            }
        }

      if (dev->recv.buffer != g_uart1rxguard.data)
        {
          ok = false;
          repaired1 = true;
          dev->recv.buffer = g_uart1rxguard.data;
          if ((uart1_diag_bits & (1u << 1)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('p');
              uart1_diag_bits |= (1u << 1);
            }
        }

      if (dev->recv.size != CONFIG_UART1_RXBUFSIZE)
        {
          ok = false;
          repaired1 = true;
          dev->recv.size = CONFIG_UART1_RXBUFSIZE;
          if ((uart1_diag_bits & (1u << 2)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('s');
              uart1_diag_bits |= (1u << 2);
            }
        }

      if (dev->recv.head >= dev->recv.size)
        {
          ok = false;
          repaired1 = true;
          dev->recv.head = 0;
          if ((uart1_diag_bits & (1u << 3)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('h');
              uart1_diag_bits |= (1u << 3);
            }
        }

      if (dev->recv.tail >= dev->recv.size)
        {
          ok = false;
          repaired1 = true;
          dev->recv.tail = 0;
          if ((uart1_diag_bits & (1u << 4)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('t');
              uart1_diag_bits |= (1u << 4);
            }
        }

      if (dev->xmit.head >= dev->xmit.size)
        {
          ok = false;
          repaired1 = true;
          dev->xmit.head = 0;
          if ((uart1_diag_bits & (1u << 9)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('m');
              uart1_diag_bits |= (1u << 9);
            }
        }

      if (dev->xmit.tail >= dev->xmit.size)
        {
          ok = false;
          repaired1 = true;
          dev->xmit.tail = 0;
          if ((uart1_diag_bits & (1u << 10)) == 0)
            {
              arm_lowputc('!');
              arm_lowputc(src);
              arm_lowputc('M');
              uart1_diag_bits |= (1u << 10);
            }
        }

      if (repaired1)
        {
          dev->ops = &g_uart_ops;
          dev->priv = &g_uart1priv;
          dev->recv.buffer = g_uart1rxguard.data;
          dev->recv.size = CONFIG_UART1_RXBUFSIZE;
          if (dev->recv.head >= dev->recv.size)
            {
              dev->recv.head = 0;
            }

          if (dev->recv.tail >= dev->recv.size)
            {
              dev->recv.tail = 0;
            }

          dev->xmit.buffer = g_uart1txbuffer;
          dev->xmit.size = CONFIG_UART1_TXBUFSIZE;
          if (dev->xmit.head >= dev->xmit.size)
            {
              dev->xmit.head = 0;
            }

          if (dev->xmit.tail >= dev->xmit.size)
            {
              dev->xmit.tail = 0;
            }
        }

      return ok;
    }
#endif

  return true;
}
#endif

/****************************************************************************
 * Inline Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_serialin
 ****************************************************************************/

static inline uint32_t up_serialin(struct up_dev_s *priv, int offset)
{
  return getreg32(priv->uartbase + offset);
}

/****************************************************************************
 * Name: up_serialout
 ****************************************************************************/

static inline void up_serialout(struct up_dev_s *priv, int offset,
                                uint32_t value)
{
  putreg32(value, priv->uartbase + offset);
}

/****************************************************************************
 * Name: up_disableuartint
 ****************************************************************************/

static inline void up_disableuartint(struct up_dev_s *priv,
                                     uint32_t *ier)
{
  irqstate_t flags;

  flags = spin_lock_irqsave(&priv->lock);
  if (ier)
    {
      *ier = priv->ier & RP2040_UART_INTR_ALL;
    }

  priv->ier &= ~RP2040_UART_INTR_ALL;
  up_serialout(priv, RP2040_UART_UARTIMSC_OFFSET, priv->ier);
  spin_unlock_irqrestore(&priv->lock, flags);
}

/****************************************************************************
 * Name: up_restoreuartint
 ****************************************************************************/

static inline void up_restoreuartint(struct up_dev_s *priv, uint32_t ier)
{
  irqstate_t flags;

  flags = spin_lock_irqsave(&priv->lock);
  priv->ier |= ier & RP2040_UART_INTR_ALL;
  up_serialout(priv, RP2040_UART_UARTIMSC_OFFSET, priv->ier);
  spin_unlock_irqrestore(&priv->lock, flags);
}

/****************************************************************************
 * Name: up_enablebreaks
 ****************************************************************************/

static inline void up_enablebreaks(struct up_dev_s *priv, bool enable)
{
  uint32_t lcr = up_serialin(priv, RP2040_UART_UARTLCR_H_OFFSET);
  if (enable)
    {
      lcr |= RP2040_UART_UARTLCR_H_BRK;
    }
  else
    {
      lcr &= ~RP2040_UART_UARTLCR_H_BRK;
    }

  up_serialout(priv, RP2040_UART_UARTLCR_H_OFFSET, lcr);
}

/****************************************************************************
 * Name: up_set_format
 *
 * Description:
 *   Set the serial line format and speed.
 *
 ****************************************************************************/

#ifndef CONFIG_SUPPRESS_UART_CONFIG
static void up_set_format(struct uart_dev_s *dev)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  uint32_t lcr;
  uint32_t cr;
  uint32_t cr_en;
  irqstate_t flags;

  flags = spin_lock_irqsave(&priv->lock);

  /* Get the original state of control register */

  cr    = up_serialin(priv, RP2040_UART_UARTCR_OFFSET);
  cr_en = cr & RP2040_UART_UARTCR_UARTEN;
  cr   &= ~RP2040_UART_UARTCR_UARTEN;

  /* Disable until the format bits and baud rate registers are updated */

  up_serialout(priv, RP2040_UART_UARTCR_OFFSET, cr);

  /* Set the BAUD divisor */

  rp2040_setbaud(priv->uartbase, priv->basefreq, priv->baud);

  /* Set up the LCR */

  lcr = up_serialin(priv, RP2040_UART_UARTLCR_H_OFFSET);

  lcr &= ~(RP2040_UART_LCR_H_WLEN(8) | RP2040_UART_UARTLCR_H_STP2 |
           RP2040_UART_UARTLCR_H_EPS | RP2040_UART_UARTLCR_H_PEN);

  if ((5 <= priv->bits) && (priv->bits < 8))
    {
      lcr |= RP2040_UART_LCR_H_WLEN(priv->bits);
    }
  else
    {
      lcr |= RP2040_UART_LCR_H_WLEN(8);
    }

  if (priv->stopbits2)
    {
      lcr |= RP2040_UART_UARTLCR_H_STP2;
    }

  if (priv->parity == 1)
    {
      lcr |= (RP2040_UART_UARTLCR_H_PEN);
    }
  else if (priv->parity == 2)
    {
      lcr |= (RP2040_UART_UARTLCR_H_PEN | RP2040_UART_UARTLCR_H_EPS);
    }

  up_serialout(priv, RP2040_UART_UARTLCR_H_OFFSET, lcr);

  /* Enable Auto-RTS and Auto-CS Flow Control in the Modem Control Register */

  cr &= ~(RP2040_UART_UARTCR_RTSEN | RP2040_UART_UARTCR_CTSEN);
  cr |= RP2040_UART_UARTCR_RTS;

#ifdef CONFIG_SERIAL_IFLOWCONTROL
  if (priv->iflow)
    {
      cr |= RP2040_UART_UARTCR_RTSEN;
    }
#endif
#ifdef CONFIG_SERIAL_OFLOWCONTROL
  if (priv->oflow)
    {
      cr |= RP2040_UART_UARTCR_CTSEN;
    }
#endif
  up_serialout(priv, RP2040_UART_UARTCR_OFFSET, cr | cr_en);

  spin_unlock_irqrestore(&priv->lock, flags);
}
#endif /* CONFIG_SUPPRESS_UART_CONFIG */

/****************************************************************************
 * Name: up_setup
 *
 * Description:
 *   Configure the UART baud, bits, parity, fifos, etc. This method is
 *   called the first time that the serial port is opened.
 *
 ****************************************************************************/

static int up_setup(struct uart_dev_s *dev)
{
#ifndef CONFIG_SUPPRESS_UART_CONFIG
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  uint32_t lcr;
  uint32_t cr;

  /* Init HW */

  up_serialout(priv, RP2040_UART_UARTCR_OFFSET, 0);
  up_serialout(priv, RP2040_UART_UARTLCR_H_OFFSET, 0);
  up_serialout(priv, RP2040_UART_UARTDMACR_OFFSET, 0);
  up_serialout(priv, RP2040_UART_UARTRSR_OFFSET, 0xf);

  /* Set up the IER */

  priv->ier = up_serialin(priv, RP2040_UART_UARTIMSC_OFFSET);

  /* Configure the UART line format and speed. */

  up_set_format(dev);

  /* Set interrupt FIFO level */

  up_serialout(priv, RP2040_UART_UARTIFLS_OFFSET, 0);

  /* Clear all interrupts */

  up_serialout(priv, RP2040_UART_UARTICR_OFFSET, 0x7ff);

  /* Enable FIFO and UART in the last */

  lcr = up_serialin(priv, RP2040_UART_UARTLCR_H_OFFSET);
  lcr |= RP2040_UART_UARTLCR_H_FEN;
  up_serialout(priv, RP2040_UART_UARTLCR_H_OFFSET, lcr);

  cr = up_serialin(priv, RP2040_UART_UARTCR_OFFSET);
  cr |= RP2040_UART_UARTCR_RXE | RP2040_UART_UARTCR_TXE |
        RP2040_UART_UARTCR_UARTEN;
  up_serialout(priv, RP2040_UART_UARTCR_OFFSET, cr);
#endif

  return OK;
}

/****************************************************************************
 * Name: up_shutdown
 *
 * Description:
 *   Disable the UART.  This method is called when the serial port is closed
 *
 ****************************************************************************/

static void up_shutdown(struct uart_dev_s *dev)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;

  /* Disable further interrupts from the UART */

  up_disableuartint(priv, NULL);
}

/****************************************************************************
 * Name: up_attach
 *
 * Description:
 *   Configure the UART to operation in interrupt driven mode.
 *   This method is called when the serial port is opened.
 *   Normally, this is just after the the setup() method is called,
 *   however, the serial console may operate in  a non-interrupt driven mode
 *   during the boot phase.
 *
 *   RX and TX interrupts are not enabled when by the attach method (unless
 *   the hardware supports multiple levels of interrupt enabling).
 *   The RX and TX interrupts are not enabled until the txint() and rxint()
 *   methods are called.
 *
 ****************************************************************************/

static int up_attach(struct uart_dev_s *dev)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  int ret;

  /* Attach and enable the IRQ */

  ret = irq_attach(priv->irq, up_interrupt, dev);
  if (ret == OK)
    {
      /* Enable the interrupt (RX and TX interrupts are still disabled
       * in the UART
       */

      up_enable_irq(priv->irq);

      /* RP2040 SMP bring-up: ensure RX interrupt source is enabled as soon
       * as the IRQ line is attached, otherwise read-side waits can starve
       * before rxint() is reached by higher layers.
       */

      up_rxint(dev, true);
    }

  return ret;
}

/****************************************************************************
 * Name: up_detach
 *
 * Description:
 *   Detach UART interrupts.  This method is called when the serial port is
 *   closed normally just before the shutdown method is called.
 *   The exception is the serial console which is never shutdown.
 *
 ****************************************************************************/

static void up_detach(struct uart_dev_s *dev)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  up_disable_irq(priv->irq);
  irq_detach(priv->irq);
}

/****************************************************************************
 * Name: up_rxflowcontrol
 *
 * Description:
 *   Called when Rx buffer is full (or exceeds configured watermark levels
 *   if CONFIG_SERIAL_IFLOWCONTROL_WATERMARKS is defined).
 *   Return true if UART activated RX flow control to block more incoming
 *   data
 *
 * Input parameters:
 *   dev       - UART device instance
 *   nbuffered - the number of characters currently buffered
 *               (if CONFIG_SERIAL_IFLOWCONTROL_WATERMARKS is
 *               not defined the value will be 0 for an empty buffer or the
 *               defined buffer size for a full buffer)
 *   upper     - true indicates the upper watermark was crossed where
 *               false indicates the lower watermark has been crossed
 *
 * Returned Value:
 *   true if RX flow control activated.
 *
 ****************************************************************************/

#ifdef CONFIG_SERIAL_IFLOWCONTROL
static bool up_rxflowcontrol(struct uart_dev_s *dev,
                             unsigned int nbuffered, bool upper)
{
  up_rxint(dev, !upper);
  return true;
}
#endif /* CONFIG_SERIAL_IFLOWCONTROL */

/****************************************************************************
 * Name: up_interrupt
 *
 * Description:
 *   This is the UART interrupt handler.  It will be invoked when an
 *   interrupt received on the 'irq'  It should call uart_transmitchars or
 *   uart_receivechar to perform the appropriate data transfers.  The
 *   interrupt handling logic must be able to map the 'irq' number into the
 *   appropriate uart_dev_s structure in order to call these functions.
 *
 ****************************************************************************/

static int up_interrupt(int irq, void *context, void *arg)
{
  struct uart_dev_s *dev = (struct uart_dev_s *)arg;
  struct up_dev_s *priv;
  uint32_t status;
  int passes;

  priv = (struct up_dev_s *)dev->priv;

  /* Loop until there are no characters to be transferred or,
   * until we have been looping for a long time.
   */

  for (passes = 0; passes < 256; passes++)
    {
      /* Get the current UART status and check for loop
       * termination conditions
       */

      status = up_serialin(priv, RP2040_UART_UARTMIS_OFFSET);
#ifdef CONFIG_ARCH_CHIP_RP2040
      arm_lowputc('m');
#endif
      if (status == 0)
        {
          return OK;
        }

      up_serialout(priv, RP2040_UART_UARTICR_OFFSET, status);
      if (status & RP2040_UART_UARTICR_RIMIC)
        {
        }

      if (status & RP2040_UART_UARTICR_CTSMIC)
        {
        }

      if (status & RP2040_UART_UARTICR_DCDMIC)
        {
        }

      if (status & RP2040_UART_UARTICR_DSRMIC)
        {
        }

      if (status & (RP2040_UART_UARTICR_RXIC | RP2040_UART_UARTICR_RTIC))
        {
#ifdef CONFIG_ARCH_CHIP_RP2040
          arm_lowputc('r');
#endif
          uart_recvchars(dev);
#ifdef CONFIG_ARCH_CHIP_RP2040
          arm_lowputc('R');
#endif
        }

      if (status & RP2040_UART_UARTICR_TXIC)
        {
          uart_xmitchars(dev);
        }

      if (status & RP2040_UART_UARTICR_FEIC)
        {
        }

      if (status & RP2040_UART_UARTICR_PEIC)
        {
        }

      if (status & RP2040_UART_UARTICR_BEIC)
        {
        }

      if (status & RP2040_UART_UARTICR_OEIC)
        {
        }
    }

  return OK;
}

/****************************************************************************
 * Name: up_ioctl
 *
 * Description:
 *   All ioctl calls will be routed through this method
 *
 ****************************************************************************/

static int up_ioctl(struct file *filep, int cmd, unsigned long arg)
{
  struct inode *inode    = filep->f_inode;
  struct uart_dev_s *dev = inode->i_private;
  struct up_dev_s *priv  = (struct up_dev_s *)dev->priv;
  int ret                = OK;

  switch (cmd)
    {
#ifdef CONFIG_SERIAL_TIOCSERGSTRUCT
      case TIOCSERGSTRUCT:
        {
          struct up_dev_s *user = (struct up_dev_s *)arg;
          if (!user)
            {
              ret = -EINVAL;
            }
          else
            {
              memcpy(user, dev, sizeof(struct up_dev_s));
            }
        }
        break;
#endif

#ifdef CONFIG_SERIAL_TERMIOS
      case TCGETS:
        {
          struct termios *termiosp = (struct termios *)arg;
          irqstate_t flags;

          if (!termiosp)
            {
              ret = -EINVAL;
              break;
            }

          flags = spin_lock_irqsave(&priv->lock);

          termiosp->c_cflag = ((priv->parity != 0) ? PARENB : 0) |
                              ((priv->parity == 1) ? PARODD : 0) |
#ifdef CONFIG_SERIAL_OFLOWCONTROL
                              ((priv->oflow) ? CCTS_OFLOW : 0) |
#endif
#ifdef CONFIG_SERIAL_IFLOWCONTROL
                              ((priv->iflow) ? CRTS_IFLOW : 0) |
#endif
                              ((priv->stopbits2) ? CSTOPB : 0);

          cfsetispeed(termiosp, priv->baud);

          switch (priv->bits)
            {
              case 5:
                termiosp->c_cflag |= CS5;
                break;

              case 6:
                termiosp->c_cflag |= CS6;
                break;

              case 7:
                termiosp->c_cflag |= CS7;
                break;

              case 8:
              default:
                termiosp->c_cflag |= CS8;
                break;
            }

          spin_unlock_irqrestore(&priv->lock, flags);
        }
        break;

      case TCSETS:
        {
          struct termios *termiosp = (struct termios *)arg;
          irqstate_t flags;

          if (!termiosp)
            {
              ret = -EINVAL;
              break;
            }

          flags = spin_lock_irqsave(&priv->lock);

          switch (termiosp->c_cflag & CSIZE)
            {
              case CS5:
                priv->bits = 5;
                break;

              case CS6:
                priv->bits = 6;
                break;

              case CS7:
                priv->bits = 7;
                break;

              case CS8:
              default:
                priv->bits = 8;
                break;
            }

          if ((termiosp->c_cflag & PARENB) != 0)
            {
              priv->parity = (termiosp->c_cflag & PARODD) ? 1 : 2;
            }
          else
            {
              priv->parity = 0;
            }

          priv->stopbits2 = (termiosp->c_cflag & CSTOPB) != 0;

#ifdef CONFIG_SERIAL_OFLOWCONTROL
          priv->oflow = (termiosp->c_cflag & CCTS_OFLOW) != 0;
#endif
#ifdef CONFIG_SERIAL_IFLOWCONTROL
          priv->iflow = (termiosp->c_cflag & CRTS_IFLOW) != 0;
#endif
          priv->baud = cfgetispeed(termiosp);

          /* Configure the UART line format and speed. */

          up_set_format(dev);

          spin_unlock_irqrestore(&priv->lock, flags);
        }
        break;
#endif

      case TIOCSBRK: /* BSD compatibility: Turn break on, unconditionally */
        {
          irqstate_t flags = spin_lock_irqsave(&priv->lock);
          up_enablebreaks(priv, true);
          spin_unlock_irqrestore(&priv->lock, flags);
        }
        break;

      case TIOCCBRK: /* BSD compatibility: Turn break off, unconditionally */
        {
          irqstate_t flags;
          flags = spin_lock_irqsave(&priv->lock);
          up_enablebreaks(priv, false);
          spin_unlock_irqrestore(&priv->lock, flags);
        }
        break;

      case TCFLSH: /* Flush TX fifo etc. */
        {
          while (!up_txempty(dev));
        }
        break;

      default:
        ret = -ENOTTY;
        break;
    }

  return ret;
}

/****************************************************************************
 * Name: up_receive
 *
 * Description:
 *   Called (usually) from the interrupt level to receive one
 *   character from the UART.  Error bits associated with the
 *   receipt are provided in the return 'status'.
 *
 ****************************************************************************/

static int up_receive(struct uart_dev_s *dev, unsigned int *status)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  uint32_t rbr;

  rbr     = up_serialin(priv, RP2040_UART_UARTDR_OFFSET);
  *status = rbr & 0xf00;
  return rbr & 0xff;
}

/****************************************************************************
 * Name: up_rxint
 *
 * Description:
 *   Call to enable or disable RX interrupts
 *
 ****************************************************************************/

static void up_rxint(struct uart_dev_s *dev, bool enable)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  irqstate_t flags;

#ifdef CONFIG_ARCH_CHIP_RP2040
  if (!enable)
    {
      arm_lowputc('R');
      arm_lowputc('X');
    }
#endif

  flags = spin_lock_irqsave(&priv->lock);
  if (enable)
    {
#ifndef CONFIG_SUPPRESS_SERIAL_INTS
      priv->ier |= (RP2040_UART_UARTIMSC_RXIM | RP2040_UART_UARTIMSC_RTIM);
#else
      arm_lowputc('-');  /* Suppressed (no RX ints!) */
#endif
    }
  else
    {
      arm_lowputc('d');  /* Disabling RX ints */
      priv->ier &= ~(RP2040_UART_UARTIMSC_RXIM | RP2040_UART_UARTIMSC_RTIM);
    }

  up_serialout(priv, RP2040_UART_UARTIMSC_OFFSET, priv->ier);
  spin_unlock_irqrestore(&priv->lock, flags);

#ifdef CONFIG_ARCH_CHIP_RP2040
  if (!enable)
    {
      arm_lowputc(')');
    }
#endif
}

/****************************************************************************
 * Name: up_rxavailable
 *
 * Description:
 *   Return true if the receive fifo is not empty
 *
 ****************************************************************************/

static bool up_rxavailable(struct uart_dev_s *dev)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  return ((up_serialin(priv, RP2040_UART_UARTFR_OFFSET)
           & RP2040_UART_UARTFR_RXFE) == 0);
}

/****************************************************************************
 * Name: up_send
 *
 * Description:
 *   This method will send one byte on the UART
 *
 ****************************************************************************/

static void up_send(struct uart_dev_s *dev, int ch)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  up_serialout(priv, RP2040_UART_UARTDR_OFFSET, (uint32_t)ch);
}

/****************************************************************************
 * Name: up_txint
 *
 * Description:
 *   Call to enable or disable TX interrupts
 *
 ****************************************************************************/

static void up_txint(struct uart_dev_s *dev, bool enable)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  irqstate_t flags;

  flags = enter_critical_section();
  if (enable)
    {
#ifndef CONFIG_SUPPRESS_SERIAL_INTS
      priv->ier |= RP2040_UART_UARTICR_TXIC;
      up_serialout(priv, RP2040_UART_UARTIMSC_OFFSET, priv->ier);

      /* Fake a TX interrupt here by just calling uart_xmitchars() with
       * interrupts disabled (note this may recurse).
       */

      uart_xmitchars(dev);
#endif
    }
  else
    {
      priv->ier &= ~RP2040_UART_UARTICR_TXIC;
      up_serialout(priv, RP2040_UART_UARTIMSC_OFFSET, priv->ier);
    }

  leave_critical_section(flags);
}

/****************************************************************************
 * Name: up_txready
 *
 * Description:
 *   Return true if the tranmsit fifo is not full
 *
 ****************************************************************************/

static bool up_txready(struct uart_dev_s *dev)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  return ((up_serialin(priv, RP2040_UART_UARTFR_OFFSET)
           & RP2040_UART_UARTFR_TXFF) == 0);
}

/****************************************************************************
 * Name: up_txempty
 *
 * Description:
 *   Return true if the transmit fifo is empty
 *
 ****************************************************************************/

static bool up_txempty(struct uart_dev_s *dev)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  uint32_t rbr = 0;
  rbr = up_serialin(priv, RP2040_UART_UARTFR_OFFSET);
  return (((rbr & RP2040_UART_UARTFR_TXFE) != 0) &&
          ((rbr & RP2040_UART_UARTFR_BUSY) == 0));
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: arm_earlyserialinit
 *
 * Description:
 *   Performs the low level UART initialization early in debug so that the
 *   serial console will be available during bootup.  This must be called
 *   before arm_serialinit.
 *
 *   NOTE: Configuration of the CONSOLE UART was performed by up_lowsetup()
 *   very early in the boot sequence.
 *
 ****************************************************************************/

#ifdef USE_EARLYSERIALINIT
void arm_earlyserialinit(void)
{
  /* Configuration whichever one is the console */

#  ifdef CONSOLE_DEV
  CONSOLE_DEV.isconsole = true;
#ifdef CONFIG_ARCH_CHIP_RP2040
  /* RP2040 console is already configured in rp2040_lowsetup(). Re-running
   * up_setup() here can disrupt low-level debug output during early boot.
   */
#else
  up_setup(&CONSOLE_DEV);
#endif
#  endif
}
#endif

/****************************************************************************
 * Name: arm_serialinit
 *
 * Description:
 *   Register serial console and serial ports.  This assumes that
 *   arm_earlyserialinit was called previously.
 *
 ****************************************************************************/

void arm_serialinit(void)
{
#ifdef CONSOLE_DEV
  uart_register("/dev/console", &CONSOLE_DEV);
#endif
#ifdef TTYS0_DEV
  uart_register("/dev/ttyS0", &TTYS0_DEV);
#endif
#ifdef TTYS1_DEV
  uart_register("/dev/ttyS1", &TTYS1_DEV);
#endif
}

/****************************************************************************
 * Name: up_putc
 *
 * Description:
 *   Provide priority, low-level access to support OS debug  writes
 *
 ****************************************************************************/

int up_putc(int ch)
{
#ifdef HAVE_CONSOLE
  struct up_dev_s *priv = (struct up_dev_s *)CONSOLE_DEV.priv;
  uint32_t ier;
  up_disableuartint(priv, &ier);
#endif

  /* Check for LF */

  if (ch == '\n')
    {
      /* Add CR */

      arm_lowputc('\r');
    }

  arm_lowputc(ch);
#ifdef HAVE_CONSOLE
  up_restoreuartint(priv, ier);
#endif

  return ch;
}

#else /* USE_SERIALDRIVER */

/****************************************************************************
 * Name: up_putc
 *
 * Description:
 *   Provide priority, low-level access to support OS debug writes
 *
 ****************************************************************************/

int up_putc(int ch)
{
#ifdef HAVE_UART
  /* Check for LF */

  if (ch == '\n')
    {
      /* Add CR */

      arm_lowputc('\r');
    }

  arm_lowputc(ch);
#endif
  return ch;
}

#endif /* USE_SERIALDRIVER */
