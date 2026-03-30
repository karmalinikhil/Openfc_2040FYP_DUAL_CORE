/****************************************************************************
 * apps/nshlib/nsh_consolemain.c
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

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include <sys/boardctl.h>

#include "nsh.h"
#include "nsh_console.h"

#include "netutils/netinit.h"

#ifdef CONFIG_ARCH_CHIP_RP2040
extern void arm_lowputc(char ch);
#  define nshcprogress(c) arm_lowputc((char)(c))
#else
#  define nshcprogress(c)
#endif

#if !defined(CONFIG_NSH_ALTCONDEV) && !defined(HAVE_USB_CONSOLE) && \
    !defined(HAVE_USB_KEYBOARD)

#ifdef CONFIG_ARCH_CHIP_RP2040
static bool nsh_rp2040_fd_can_read(int fd)
{
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0 || (flags & O_ACCMODE) == O_WRONLY)
    {
      return false;
    }

  /* Keep this check non-blocking and descriptor-only to avoid open/read-side
   * stalls in sensitive RP2040 SMP bring-up paths.
   */

  return true;
}

static int nsh_rp2040_rebind_stdio(FAR struct console_stdio_s *pstate)
{
  int ret;
  int fd;

  /* First try rebinding with existing stdio descriptors to avoid any
   * open-time stalls in RP2040 SMP bring-up.
   */

  nshcprogress('a');
  ret = -1;

  if (nsh_rp2040_fd_can_read(0))
    {
      nshcprogress('B');
      ret = OK;
    }
  else if (nsh_rp2040_fd_can_read(1))
    {
      nshcprogress('C');
      ret = dup2(1, 0);
      nshcprogress('D');
    }

  if (ret < 0 || !nsh_rp2040_fd_can_read(0))
    {
      nshcprogress('b');
      ret = nsh_rp2040_fd_can_read(2) ? dup2(2, 0) : -1;
      nshcprogress('E');
    }

  if (ret >= 0 && nsh_rp2040_fd_can_read(0))
    {
      nshcprogress('F');
      nshcprogress('O');
      return OK;
    }

  /* RP2040 recovery fast-path: avoid stream-backed fallback probes here.
   * In this branch, they can stall before session entry (observed around
   * marker 'd'). Continue with inherited stdio and let nsh_session perform
   * bounded descriptor probing/recovery.
   */

  nshcprogress('u');
  return OK;

  /* Next try stream-backed descriptors from console state. */

  nshcprogress('c');
  fd = fileno(pstate->cn_outstream);
  if (fd >= 0 && nsh_rp2040_fd_can_read(fd))
    {
      nshcprogress('G');
      ret = dup2(fd, 0);
      nshcprogress('H');
    }
  else
    {
      nshcprogress('J');
      ret = -1;
    }

  if (ret < 0)
    {
      nshcprogress('d');
      fd = fileno(pstate->cn_errstream);
      if (fd >= 0 && nsh_rp2040_fd_can_read(fd))
        {
          nshcprogress('K');
          ret = dup2(fd, 0);
          nshcprogress('L');
        }
    }

  if (ret >= 0 && !nsh_rp2040_fd_can_read(0))
    {
      ret = -1;
    }

  /* Do not scan inherited fd range on RP2040 here. Wide scans create long
   * noisy loops and have not recovered stdin on this branch.
   */

  if (ret >= 0 && !nsh_rp2040_fd_can_read(0))
    {
      ret = -1;
    }

  /* Avoid open-based fallback in this RP2040 branch: it can deadlock in
   * the same inode/driver paths we are trying to escape. Continue with the
   * current inherited stdio state if rebinding could not be improved.
   */

  if (ret < 0)
    {
      nshcprogress('u');
      return OK;
    }

  if (ret < 0)
    {
      nshcprogress('e');
      return -ENODEV;
    }

  if (dup2(0, 1) < 0 || dup2(0, 2) < 0)
    {
      nshcprogress('i');
      return -EIO;
    }

  nshcprogress('N');
  nshcprogress('j');
  clearerr(stdin);
  clearerr(stdout);
  clearerr(stderr);
  nshcprogress('h');
  return OK;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nsh_consolemain (Normal character device version)
 *
 * Description:
 *   This interface may be to called or started with task_start to start a
 *   single an NSH instance that operates on stdin and stdout.  This
 *   function does not normally return (see below).
 *
 *   This version of nsh_consolemain handles generic /dev/console character
 *   devices (see nsh_usbconsole.c and usb_usbkeyboard for other versions
 *   for special USB console devices).
 *
 * Input Parameters:
 *   Standard task start-up arguments.  These are not used.  argc may be
 *   zero and argv may be NULL.
 *
 * Returned Values:
 *   This function does not normally return.  exit() is usually called to
 *   terminate the NSH session.  This function will return in the event of
 *   an error.  In that case, a non-zero value is returned (EXIT_FAILURE=1).
 *
 ****************************************************************************/

int nsh_consolemain(int argc, FAR char *argv[])
{
  nshcprogress('5');
  FAR struct console_stdio_s *pstate = nsh_newconsole(true);
  int ret;

  nshcprogress('6');

  DEBUGASSERT(pstate != NULL);
  if (pstate == NULL)
    {
      return -ENOMEM;
    }

#ifdef CONFIG_NSH_USBDEV_TRACE
  /* Initialize any USB tracing options that were requested */

  usbtrace_enable(TRACE_BITSET);
#endif

#if defined(CONFIG_NSH_ROMFSETC) && !defined(CONFIG_NSH_DISABLESCRIPT)
  /* Execute the system init script */

#ifndef CONFIG_ARCH_CHIP_RP2040
  nsh_sysinitscript(&pstate->cn_vtbl);
#endif
#endif

#ifdef CONFIG_NSH_NETINIT
  /* Bring up the network */

  netinit_bringup();
#endif

  nshcprogress('7');

#if defined(CONFIG_NSH_ARCHINIT) && defined(CONFIG_BOARDCTL_FINALINIT)
  /* Perform architecture-specific final-initialization (if configured) */

  boardctl(BOARDIOC_FINALINIT, 0);
#endif

#if defined(CONFIG_NSH_ROMFSETC) && !defined(CONFIG_NSH_DISABLESCRIPT)
  /* Execute the start-up script */

#ifndef CONFIG_ARCH_CHIP_RP2040
  nsh_initscript(&pstate->cn_vtbl);
#endif
#endif

  /* Execute the session */

#ifdef CONFIG_ARCH_CHIP_RP2040
  nshcprogress('9');
  ret = nsh_rp2040_rebind_stdio(pstate);
  if (ret < 0)
    {
      nshcprogress('f');
    }
  else
    {
      nshcprogress('A');
    }
#endif

  ret = nsh_session(pstate, true, argc, argv);
  nshcprogress('8');

  /* Exit upon return */

  nsh_exit(&pstate->cn_vtbl, ret);
  return ret;
}

#endif /* !HAVE_USB_CONSOLE && !HAVE_USB_KEYBOARD !HAVE_SLCD_CONSOLE */
