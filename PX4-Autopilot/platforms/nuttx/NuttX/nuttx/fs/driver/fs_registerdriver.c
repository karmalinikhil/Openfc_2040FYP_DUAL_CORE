/****************************************************************************
 * fs/driver/fs_registerdriver.c
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
#include <errno.h>
#include <string.h>

#include <nuttx/fs/fs.h>

#include "inode/inode.h"

#ifdef CONFIG_ARCH_CHIP_RP2040
extern void arm_lowputc(char ch);
#  define regdrvprogress(c) arm_lowputc((char)(c))
#else
#  define regdrvprogress(c)
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: register_driver
 *
 * Description:
 *   Register a character driver inode the pseudo file system.
 *
 * Input Parameters:
 *   path - The path to the inode to create
 *   fops - The file operations structure
 *   mode - inmode privileges
 *   priv - Private, user data that will be associated with the inode.
 *
 * Returned Value:
 *   Zero on success (with the inode point in 'inode'); A negated errno
 *   value is returned on a failure (all error values returned by
 *   inode_reserve):
 *
 *   EINVAL - 'path' is invalid for this operation
 *   EEXIST - An inode already exists at 'path'
 *   ENOMEM - Failed to allocate in-memory resources for the operation
 *
 ****************************************************************************/

int register_driver(FAR const char *path,
                    FAR const struct file_operations *fops,
                    mode_t mode, FAR void *priv)
{
  FAR struct inode *node;
  bool have_lock = false;
  int ret;

  regdrvprogress('d');

#ifdef CONFIG_ARCH_CHIP_RP2040
  /* RP2040 diagnostic path: force lock-free reserve for critical serial
   * nodes so registration cannot stall on inode lock handoff/unlock.
   */

  if (strcmp(path, "/dev/console") == 0 ||
      strcmp(path, "/dev/ttyS0") == 0 ||
      strcmp(path, "/dev/ttyS1") == 0 ||
      strcmp(path, "/dev/null") == 0)
    {
      regdrvprogress('x');
      ret = inode_reserve(path, mode, &node);
      regdrvprogress('y');
      if (ret >= 0)
        {
          INODE_SET_DRIVER(node);
          node->u.i_ops   = fops;
          node->i_private = priv;
          return OK;
        }

      return ret;
    }
#endif

  /* Insert a dummy node -- we need to hold the inode semaphore because we
   * will have a momentarily bad structure.
   */

#ifdef CONFIG_ARCH_CHIP_RP2040
  /* RP2040 SMP diagnostic path: avoid indefinite blocking on inode lock for
   * tracked serial device-node registration. Caller can retry on -EAGAIN.
   */

  if (strcmp(path, "/dev/console") == 0 ||
      strcmp(path, "/dev/ttyS0") == 0 ||
      strcmp(path, "/dev/ttyS1") == 0 ||
      strcmp(path, "/dev/null") == 0)
    {
      regdrvprogress('t');
      ret = inode_semtrytake();
      if (ret < 0)
        {
          regdrvprogress('g');
          /* Last-resort diagnostic fallback: reserve node without taking
           * inode lock so bring-up can progress and expose next blockers.
           */

          regdrvprogress('h');
          ret = inode_reserve(path, mode, &node);
          if (ret >= 0)
            {
              regdrvprogress('i');
              INODE_SET_DRIVER(node);
              node->u.i_ops   = fops;
              node->i_private = priv;
              ret             = OK;
            }

          return ret;
        }

      regdrvprogress('l');
      have_lock = true;
    }
  else
#endif
    {
      regdrvprogress('s');
      ret = inode_semtake();
      regdrvprogress('S');
      have_lock = ret >= 0;
    }

  if (ret < 0)
    {
      return ret;
    }

  ret = inode_reserve(path, mode, &node);
  regdrvprogress('r');
  if (ret >= 0)
    {
      regdrvprogress('v');
      /* We have it, now populate it with driver specific information.
       * NOTE that the initial reference count on the new inode is zero.
       */

      INODE_SET_DRIVER(node);

      node->u.i_ops   = fops;
      node->i_private = priv;
      ret             = OK;
    }

  if (have_lock)
    {
      regdrvprogress('u');
      inode_semgive();
    }

  return ret;
}
