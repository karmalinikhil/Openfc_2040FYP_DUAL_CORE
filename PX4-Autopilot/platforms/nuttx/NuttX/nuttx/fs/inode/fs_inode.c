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

#include <assert.h>
#include <errno.h>

#include <nuttx/fs/fs.h>
#include <nuttx/mutex.h>

#include "inode/inode.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Use a simple semaphore instead of recursive mutex for RP2040 SMP stability */
#ifdef CONFIG_ARCH_CHIP_RP2040
static sem_t g_inode_sem = SEM_INITIALIZER(1);
#else
static rmutex_t g_inode_lock = NXRMUTEX_INITIALIZER;
#endif

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
#ifdef CONFIG_ARCH_CHIP_RP2040
  /* RP2040: Use simple semaphore to avoid recursive mutex SMP issues */
  return nxsem_wait(&g_inode_sem);
#else
  up_puts("MARKER_INODE_SEMTAKE_START");
  
  /* Debug: Check if g_inode_lock structure looks sane */
  up_puts("MARKER_CHECKING_INODE_LOCK");
  if (g_inode_lock.holder == NXRMUTEX_NO_HOLDER && g_inode_lock.count == 0)
    {
      up_puts("MARKER_INODE_LOCK_LOOKS_GOOD");
    }
  else
    {
      up_puts("MARKER_INODE_LOCK_CORRUPTED");
    }
    
  up_puts("MARKER_BEFORE_NXRMUTEX_LOCK_CALL");
  int ret = nxrmutex_lock(&g_inode_lock);
  up_puts("MARKER_INODE_SEMTAKE_END");
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
  /* RP2040: Use simple semaphore to avoid recursive mutex SMP issues */
  return nxsem_trywait(&g_inode_sem);
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
  /* RP2040: Use simple semaphore to avoid recursive mutex SMP issues */
  nxsem_post(&g_inode_sem);
#else
  up_puts("MARKER_INODE_SEMGIVE_START");
  DEBUGVERIFY(nxrmutex_unlock(&g_inode_lock));
  up_puts("MARKER_INODE_SEMGIVE_END");
#endif
}
