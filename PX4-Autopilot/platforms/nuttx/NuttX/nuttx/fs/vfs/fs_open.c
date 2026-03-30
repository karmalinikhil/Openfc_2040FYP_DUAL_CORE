/****************************************************************************
 * fs/vfs/fs_open.c
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
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sched.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>

#include <nuttx/cancelpt.h>
#include <nuttx/fs/fs.h>

#include "inode/inode.h"
#include "driver/driver.h"

#ifdef CONFIG_ARCH_CHIP_RP2040
extern void arm_lowputc(char ch);
#  define openprogress(c) arm_lowputc((char)(c))
#else
#  define openprogress(c)
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: file_vopen
 ****************************************************************************/

static int file_vopen(FAR struct file *filep, FAR const char *path,
                      int oflags, mode_t umask, va_list ap)
{
  struct inode_search_s desc;
  FAR struct inode *inode;
  bool track_console_open;
  bool track_ttys0_open;
#ifndef CONFIG_DISABLE_MOUNTPOINT
  mode_t mode = 0666;
#endif
  int ret;

  if (path == NULL)
    {
      return -EINVAL;
    }

  track_ttys0_open = (strcmp(path, "/dev/ttyS0") == 0);
  track_console_open = (strcmp(path, "/dev/console") == 0 ||
                        track_ttys0_open);
  if (track_console_open)
    {
      openprogress('f');
    }

#ifndef CONFIG_DISABLE_MOUNTPOINT

  /* If the file is opened for creation, then get the mode bits */

  if ((oflags & (O_WRONLY | O_CREAT)) != 0)
    {
      mode = va_arg(ap, mode_t);
    }

  mode &= ~umask;
#endif

  /* Get an inode for this file */

  SETUP_SEARCH(&desc, path, false);

  if (track_console_open && (oflags & O_NONBLOCK) != 0)
    {
#ifdef CONFIG_ARCH_CHIP_RP2040
      /* RP2040 late stdio recovery can stall inside inode_semtrytake()
       * even for the direct UART path. For the tracked non-blocking ttyS0
       * probe, bypass the trylock entirely and use the existing diagnostic
       * lockless lookup path so we can determine whether the lock itself is
       * the remaining blocker.
       */

      if (track_ttys0_open)
        {
          openprogress('y');
          ret = inode_search(&desc);
          if (ret >= 0)
            {
              desc.node->i_crefs++;
              openprogress('k');
            }
          else
            {
              openprogress('K');
            }
        }
      else
#endif
        {
      ret = inode_semtrytake();
      if (ret < 0)
        {
#ifdef CONFIG_ARCH_CHIP_RP2040
          /* RP2040 SMP bring-up workaround: if inode lock appears stuck,
           * permit a lockless lookup for tracked console non-blocking opens
           * so we can validate whether lock contention is the stdin blocker.
           */

          if (track_console_open)
            {
              openprogress(track_ttys0_open ? 'z' : 'Z');
              ret = inode_search(&desc);
              if (ret >= 0)
                {
                  desc.node->i_crefs++;
                }
            }
          else
#endif
            {
          ret = -EAGAIN;
          goto errout_with_search;
            }
        }

      if (ret == OK)
        {
          ret = inode_search(&desc);
          if (ret >= 0)
            {
              desc.node->i_crefs++;
            }

          inode_semgive();
        }
        }
    }
  else
    {
      ret = inode_find(&desc);
    }
  if (track_console_open)
    {
      openprogress('g');
    }

  if (ret < 0)
    {
      if (track_console_open)
        {
          if (ret == -EAGAIN)
            {
              openprogress('A');
            }
          else if (ret == -ENOENT)
            {
              openprogress('N');
            }
          else if (ret == -ENODEV)
            {
              openprogress('V');
            }
          else
            {
              openprogress('R');
            }
        }

      /* "O_CREAT is not set and the named file does not exist.  Or, a
       * directory component in pathname does not exist or is a dangling
       * symbolic link."
       */

      goto errout_with_search;
    }

  /* Get the search results */

  inode = desc.node;
  DEBUGASSERT(inode != NULL);

#if defined(CONFIG_BCH) && \
    !defined(CONFIG_DISABLE_MOUNTPOINT) && \
    !defined(CONFIG_DISABLE_PSEUDOFS_OPERATIONS)
  /* If the inode is block driver, then we may return a character driver
   * proxy for the block driver.  block_proxy() will instantiate a BCH
   * character driver wrapper around the block driver, open(), then
   * unlink() the character driver.
   *
   * NOTE: This will recurse to open the character driver proxy.
   */

  if (INODE_IS_BLOCK(inode) || INODE_IS_MTD(inode))
    {
      /* Release the inode reference */

      inode_release(inode);
      RELEASE_SEARCH(&desc);

      /* Get the file structure of the opened character driver proxy */

      return block_proxy(filep, path, oflags);
    }
#endif

  /* Make sure that the inode supports the requested access */

  ret = inode_checkflags(inode, oflags);
  if (ret < 0)
    {
      goto errout_with_inode;
    }

  /* Associate the inode with a file structure */

  filep->f_oflags = oflags;
  filep->f_pos    = 0;
  filep->f_inode  = inode;
  filep->f_priv   = NULL;

  /* Perform the driver open operation.  NOTE that the open method may be
   * called many times.  The driver/mountpoint logic should handled this
   * because it may also be closed that many times.
   */

  if (oflags & O_DIRECTORY)
    {
      ret = dir_allocate(filep, desc.relpath);
    }
#ifndef CONFIG_DISABLE_MOUNTPOINT
  else if (INODE_IS_MOUNTPT(inode))
    {
      if (inode->u.i_mops->open != NULL)
        {
          ret = inode->u.i_mops->open(filep, desc.relpath, oflags, mode);
        }
    }
#endif
  else if (INODE_IS_DRIVER(inode))
    {
      if (inode->u.i_ops->open != NULL)
        {
          if (track_console_open)
            {
              openprogress('h');
            }

          ret = inode->u.i_ops->open(filep);

          if (track_console_open)
            {
              openprogress('i');
            }
        }
    }
  else
    {
      ret = -ENXIO;
    }

  if (ret < 0)
    {
      if (track_console_open)
        {
          openprogress('j');

          if (ret == -EAGAIN)
            {
              openprogress('a');
            }
          else if (ret == -ENODEV)
            {
              openprogress('v');
            }
          else if (ret == -ENXIO)
            {
              openprogress('x');
            }
          else
            {
              openprogress('r');
            }
        }

      goto errout_with_inode;
    }

  RELEASE_SEARCH(&desc);
  return OK;

errout_with_inode:
  filep->f_inode = NULL;
  inode_release(inode);

errout_with_search:
  RELEASE_SEARCH(&desc);
  return ret;
}

/****************************************************************************
 * Name: nx_vopen
 ****************************************************************************/

static int nx_vopen(FAR const char *path, int oflags, va_list ap)
{
  struct file filep;
  int ret;
  int fd;

  /* Let file_vopen() do all of the work */

  ret = file_vopen(&filep, path, oflags, getumask(), ap);
  if (ret < 0)
    {
      return ret;
    }

#ifdef CONFIG_ARCH_CHIP_RP2040
  if (strcmp(path, "/dev/ttyS0") == 0 || strcmp(path, "/dev/console") == 0)
    {
      openprogress('L');
    }
#endif

  /* Allocate a new file descriptor for the inode */

  fd = files_allocate(filep.f_inode, filep.f_oflags,
                      filep.f_pos, filep.f_priv, 0);
  if (fd < 0)
    {
#ifdef CONFIG_ARCH_CHIP_RP2040
      if (strcmp(path, "/dev/ttyS0") == 0 || strcmp(path, "/dev/console") == 0)
        {
          openprogress('l');
        }
#endif
      file_close(&filep);
      return fd;
    }

#ifdef CONFIG_ARCH_CHIP_RP2040
  if (strcmp(path, "/dev/ttyS0") == 0 || strcmp(path, "/dev/console") == 0)
    {
      openprogress('D');
    }
#endif

  return fd;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: inode_checkflags
 *
 * Description:
 *   Check if the access described by 'oflags' is supported on 'inode'
 *
 ****************************************************************************/

int inode_checkflags(FAR struct inode *inode, int oflags)
{
  if (INODE_IS_PSEUDODIR(inode))
    {
      return OK;
    }

  if (inode->u.i_ops == NULL)
    {
      return -ENXIO;
    }

  if (((oflags & O_RDOK) != 0 && !inode->u.i_ops->read) ||
      ((oflags & O_WROK) != 0 && !inode->u.i_ops->write))
    {
      return -EACCES;
    }
  else
    {
      return OK;
    }
}

/****************************************************************************
 * Name: file_open
 *
 * Description:
 *   file_open() is similar to the standard 'open' interface except that it
 *   returns an instance of 'struct file' rather than a file descriptor.  It
 *   also is not a cancellation point and does not modify the errno variable.
 *
 * Input Parameters:
 *   filep  - The caller provided location in which to return the 'struct
 *            file' instance.
 *   path   - The full path to the file to be open.
 *   oflags - open flags
 *   ...    - Variable number of arguments, may include 'mode_t mode'
 *
 * Returned Value:
 *   Zero (OK) is returned on success.  On failure, a negated errno value is
 *   returned.
 *
 ****************************************************************************/

int file_open(FAR struct file *filep, FAR const char *path, int oflags, ...)
{
  va_list ap;
  int ret;

  va_start(ap, oflags);
  ret = file_vopen(filep, path, oflags, 0, ap);
  va_end(ap);

  return ret;
}

/****************************************************************************
 * Name: nx_open
 *
 * Description:
 *   nx_open() is similar to the standard 'open' interface except that is is
 *   not a cancellation point and it does not modify the errno variable.
 *
 *   nx_open() is an internal NuttX interface and should not be called from
 *   applications.
 *
 * Returned Value:
 *   The new file descriptor is returned on success; a negated errno value is
 *   returned on any failure.
 *
 ****************************************************************************/

int nx_open(FAR const char *path, int oflags, ...)
{
  va_list ap;
  int fd;

  /* Let nx_vopen() do all of the work */

  va_start(ap, oflags);
  fd = nx_vopen(path, oflags, ap);
  va_end(ap);

  return fd;
}

/****************************************************************************
 * Name: open
 *
 * Description:
 *   Standard 'open' interface
 *
 * Returned Value:
 *   The new file descriptor is returned on success; -1 (ERROR) is returned
 *   on any failure the errno value set appropriately.
 *
 ****************************************************************************/

int open(FAR const char *path, int oflags, ...)
{
  va_list ap;
  int fd;

  /* open() is a cancellation point */

  enter_cancellation_point();

  /* Let nx_vopen() do most of the work */

  va_start(ap, oflags);
  fd = nx_vopen(path, oflags, ap);
  va_end(ap);

  /* Set the errno value if any errors were reported by nx_open() */

  if (fd < 0)
    {
      set_errno(-fd);
      fd = ERROR;
    }

  leave_cancellation_point();
  return fd;
}
