/****************************************************************************
 * fs/inode/fs_inodeaddref.c
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

#include <errno.h>
#include <nuttx/fs/fs.h>
#include "inode/inode.h"

#ifdef CONFIG_ARCH_CHIP_RP2040
extern void arm_lowputc(char ch);
#  define addrefprogress(c) arm_lowputc((char)(c))
#else
#  define addrefprogress(c)
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: inode_addref
 *
 * Description:
 *   Increment the reference count on an inode (as when a file descriptor
 *   is dup'ed).
 *
 ****************************************************************************/

int inode_addref(FAR struct inode *inode)
{
  int ret = OK;

  if (inode)
    {
      /* Use blocking semaphore for reliable reference counting on all platforms */
      ret = inode_semtake();
      if (ret >= 0)
        {
          inode->i_crefs++;
          
          /* Memory barrier for dual-core visibility (RP2040 has no cache coherency) */
          __asm__ __volatile__ ("dsb sy" ::: "memory");
          
          inode_semgive();
        }
    }

  return ret;
}
