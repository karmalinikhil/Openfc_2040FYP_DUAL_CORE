/****************************************************************************
 * fs/inode/fs_inode.c
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

#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <nuttx/irq.h>
#include <nuttx/fs/fs.h>
#include <nuttx/mutex.h>
#include <nuttx/sched.h>

#include "inode/inode.h"

#ifdef CONFIG_ARCH_CHIP_RP2040
extern void arm_lowputc(char ch);
#  define inodesemprogress(c) arm_lowputc((char)(c))

static inline void inodesem_hex4(unsigned int v)
{
  static const char hex[] = "0123456789ABCDEF";
  inodesemprogress(hex[v & 0x0f]);
}

static inline void inodesem_tid4(void)
{
  inodesem_hex4((unsigned int)gettid());
}

static inline bool inodesem_owner_hint(pid_t pid)
{
  FAR struct tcb_s *tcb = nxsched_get_tcb(pid);

  inodesemprogress('P');
  inodesem_hex4((unsigned int)pid);

  if (tcb != NULL)
    {
      inodesemprogress('S');
      inodesem_hex4((unsigned int)tcb->task_state);
#if CONFIG_TASK_NAME_SIZE > 0
      inodesemprogress('N');
      if (tcb->name[0] >= 32 && tcb->name[0] <= 126)
        {
          inodesemprogress(tcb->name[0]);
        }
      else
        {
          inodesemprogress('.');
        }
#endif
      return true;
    }
  else
    {
      inodesemprogress('s');
      return false;
    }
}
#else
#  define inodesemprogress(c)
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static rmutex_t g_inode_lock = NXRMUTEX_INITIALIZER;

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: inode_initialize
 *
 * Description:
 *   This is called from the OS initialization logic to configure the file
 *   system.
 *
 ****************************************************************************/

void inode_initialize(void)
{
  /* Reserve the root node */

  inode_root_reserve();
}

/****************************************************************************
 * Name: inode_semtake
 *
 * Description:
 *   Get exclusive access to the in-memory inode tree (g_inode_sem).
 *
 ****************************************************************************/

int inode_semtake(void)
{
  int ret;

#ifdef CONFIG_ARCH_CHIP_RP2040
  for (int tries = 0; tries < 80; tries++)
    {
      pid_t tid = gettid();
      irqstate_t flags;

      flags = enter_critical_section();

      if (g_inode_lock.holder == tid)
        {
          if (g_inode_lock.count < UINT16_MAX)
            {
              g_inode_lock.count++;
            }

          ret = OK;
        }
      else if (g_inode_lock.count == 0 ||
               g_inode_lock.holder == NXRMUTEX_NO_HOLDER)
        {
          g_inode_lock.holder = tid;
          g_inode_lock.count = 1;
          if (g_inode_lock.mutex.semcount > 0)
            {
              g_inode_lock.mutex.semcount--;
            }

          ret = OK;
        }
      else
        {
          ret = -EAGAIN;
        }

      leave_critical_section(flags);
      if (ret >= 0)
        {
          return ret;
        }

      if (ret != -EAGAIN)
        {
          inodesemprogress('W');
          return ret;
        }

      if ((tries % 16) == 15)
        {
          inodesemprogress('V');
          inodesemprogress('H');
          inodesem_hex4((unsigned int)g_inode_lock.holder);
          inodesemprogress('C');
          inodesem_hex4((unsigned int)g_inode_lock.count);
          if (!inodesem_owner_hint(g_inode_lock.holder))
            {
              irqstate_t rflags = enter_critical_section();
              g_inode_lock.holder = NXRMUTEX_NO_HOLDER;
              g_inode_lock.count = 0;
              g_inode_lock.mutex.semcount = 1;
              leave_critical_section(rflags);
              inodesemprogress('R');
            }
        }

      usleep(1000);
    }

  inodesemprogress('X');
  return -EAGAIN;
#else
  ret = nxrmutex_lock(&g_inode_lock);
  return ret;
#endif
}

/****************************************************************************
 * Name: inode_semtrytake
 *
 * Description:
 *   Attempt to get exclusive access to the in-memory inode tree without
 *   blocking.
 *
 ****************************************************************************/

int inode_semtrytake(void)
{
#ifdef CONFIG_ARCH_CHIP_RP2040
  pid_t tid = gettid();
  irqstate_t flags = enter_critical_section();
  int ret;

  if (g_inode_lock.holder == tid)
    {
      if (g_inode_lock.count < UINT16_MAX)
        {
          g_inode_lock.count++;
        }

      ret = OK;
    }
  else if (g_inode_lock.count == 0 ||
           g_inode_lock.holder == NXRMUTEX_NO_HOLDER)
    {
      g_inode_lock.holder = tid;
      g_inode_lock.count = 1;
      if (g_inode_lock.mutex.semcount > 0)
        {
          g_inode_lock.mutex.semcount--;
        }

      ret = OK;
    }
  else
    {
      ret = -EAGAIN;
    }

  leave_critical_section(flags);
  return ret;
#else
  return nxrmutex_trylock(&g_inode_lock);
#endif
}

/****************************************************************************
 * Name: inode_semgive
 *
 * Description:
 *   Relinquish exclusive access to the in-memory inode tree (g_inode_sem).
 *
 ****************************************************************************/

void inode_semgive(void)
{
#ifdef CONFIG_ARCH_CHIP_RP2040
  irqstate_t flags = enter_critical_section();

  if (g_inode_lock.count > 1)
    {
      g_inode_lock.count--;
      leave_critical_section(flags);
      return;
    }

  if (g_inode_lock.count == 1)
    {
      g_inode_lock.count = 0;
      g_inode_lock.holder = NXRMUTEX_NO_HOLDER;
      g_inode_lock.mutex.semcount = 1;

      leave_critical_section(flags);
      return;
    }

  leave_critical_section(flags);
#else
  DEBUGVERIFY(nxrmutex_unlock(&g_inode_lock));
#endif
}
