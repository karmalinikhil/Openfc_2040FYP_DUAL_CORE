/****************************************************************************
 * apps/nshlib/nsh_session.c
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <assert.h>
#include <poll.h>
#include <syslog.h>

#ifdef CONFIG_ARCH_CHIP_RP2040
extern void arm_lowputc(char ch);
#  define nshprogress(c) arm_lowputc((char)(c))

static bool nsh_rp2040_fd_usable_input(int fd)
{
  int flags = fcntl(fd, F_GETFL, 0);

  /* A failed F_GETFL means the descriptor is not usable here.
   * Treating that as readable causes endless EBADF/EIO retry loops.
   */
  if (flags < 0 || (flags & O_ACCMODE) == O_WRONLY)
    {
      return false;
    }

  return true;
}
#else
#  define nshprogress(c)
#endif

#ifdef CONFIG_NSH_CLE
#  include "system/cle.h"
#else
#  include "system/readline.h"
#endif

#include "nsh.h"
#include "nsh_console.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nsh_session
 *
 * Description:
 *   This is the common session login on any NSH session.  This function
 *   returns when an error reading from the input stream occurs, presumably
 *   signaling the end of the session.
 *
 *   This function:
 *   - Performs the login sequence if so configured
 *   - Executes the NSH login script
 *   - Presents a greeting
 *   - Then provides a prompt then gets and processes the command line.
 *   - This continues until an error occurs, then the session returns.
 *
 * Input Parameters:
 *   pstate - Abstracts the underlying session.
 *
 * Returned Values:
 *   EXIT_SUCCESS or EXIT_FAILURE is returned.
 *
 ****************************************************************************/

int nsh_session(FAR struct console_stdio_s *pstate,
                bool login, int argc, FAR char *argv[])
{
  FAR struct nsh_vtbl_s *vtbl;
  int ret = EXIT_FAILURE;
  int infd;
  int flags;
#ifdef CONFIG_ARCH_CHIP_RP2040
  int eof_retry_count = 0;  /* Counter to prevent infinite retry loops */
#endif
#ifndef CONFIG_ARCH_CHIP_RP2040
  struct termios tio;
#endif

  DEBUGASSERT(pstate);
  vtbl = &pstate->cn_vtbl;
  nshprogress('a');

  /* Prefer stdin (fd0), but if it is invalid or write-only, fall back to
   * whichever console stream fd is readable.
   */

  infd = INFD(pstate);
  nshprogress('b');
  flags = fcntl(infd, F_GETFL, 0);
  nshprogress('B');
#ifdef CONFIG_ARCH_CHIP_RP2040
  if (flags < 0 || (flags & O_ACCMODE) == O_WRONLY ||
      !nsh_rp2040_fd_usable_input(infd))
#else
  if (flags < 0 || (flags & O_ACCMODE) == O_WRONLY ||
      tcgetattr(infd, &tio) < 0)
#endif
    {
      nshprogress('c');
      int outfd = OUTFD(pstate);
      nshprogress('C');

      flags = fcntl(outfd, F_GETFL, 0);
      nshprogress('D');
#ifdef CONFIG_ARCH_CHIP_RP2040
      if (nsh_rp2040_fd_usable_input(outfd))
#else
      if (flags >= 0 && (flags & O_ACCMODE) != O_WRONLY &&
          tcgetattr(outfd, &tio) == 0)
#endif
        {
          infd = outfd;
          nshprogress('n');
        }
      else
        {
          int errfd = ERRFD(pstate);
          nshprogress('E');

          flags = fcntl(errfd, F_GETFL, 0);
          nshprogress('F');
#ifdef CONFIG_ARCH_CHIP_RP2040
      if (nsh_rp2040_fd_usable_input(errfd))
#else
          if (flags >= 0 && (flags & O_ACCMODE) != O_WRONLY &&
              tcgetattr(errfd, &tio) == 0)
#endif
            {
              infd = errfd;
              nshprogress('N');
            }
        }

#ifdef CONFIG_ARCH_CHIP_RP2040
      /* Avoid wide inherited-fd scans on RP2040 SMP debug builds.
       * They create long !? floods and delay prompt/read cycles while not
       * improving recovery on this branch.
       */
#endif
    }

  nshprogress('f');

  /* readline_fd treats any non-EINTR read error as EOF. Ensure the selected
   * input descriptor is in blocking mode so a temporary lack of data does not
   * terminate the shell session.
   */

  flags = fcntl(infd, F_GETFL, 0);
  nshprogress('g');
  if (flags >= 0 && (flags & O_NONBLOCK) != 0)
    {
      nshprogress('G');
#ifdef CONFIG_ARCH_CHIP_RP2040
      /* RP2040: this F_SETFL path has repeatedly stalled at marker G.
       * Skip the nonblocking-clear operation here and continue into session
       * flow so we can progress to prompt/read handling.
       */
      nshprogress('Q');
#else
      if (fcntl(infd, F_SETFL, flags & ~O_NONBLOCK) >= 0)
        {
          nshprogress('w');
        }
      else
        {
          nshprogress('W');
        }
#endif
    }
  else
    {
      nshprogress('H');
    }

  if (login)
    {
      nshprogress('i');
#ifdef CONFIG_NSH_CONSOLE_LOGIN
      /* Login User and Password Check */

      if (nsh_login(pstate) != OK)
        {
          nsh_exit(vtbl, 1);
          return -1; /* nsh_exit does not return */
        }
#endif /* CONFIG_NSH_CONSOLE_LOGIN */

      /* Present a greeting and possibly a Message of the Day (MOTD) */

      fputs(g_nshgreeting, pstate->cn_outstream);

#ifdef CONFIG_NSH_MOTD
# ifdef CONFIG_NSH_PLATFORM_MOTD
      /* Output the platform message of the day */

      platform_motd(vtbl->iobuffer, IOBUFFERSIZE);
      fprintf(pstate->cn_outstream, "%s\n", vtbl->iobuffer);

# else
      /* Output the fixed message of the day */

      fprintf(pstate->cn_outstream, "%s\n", g_nshmotd);
# endif
#endif

      fflush(pstate->cn_outstream);
      nshprogress('g');

      /* Execute the login script */

#ifdef CONFIG_NSH_ROMFSRC
      nsh_loginscript(vtbl);
#endif
    }

  /* Process the command line option */

  if (argc > 1)
    {
      if (strcmp(argv[1], "-h") == 0)
        {
          nsh_output(vtbl, "Usage: %s [<script-path>|-c <command>]\n",
                     argv[0]);
          return EXIT_SUCCESS;
        }
      else if (strcmp(argv[1], "-c") == 0)
        {
          /* Process the inline command */

          if (argc > 2)
            {
              return nsh_parse(vtbl, argv[2]);
            }
          else
            {
              nsh_error(vtbl, g_fmtargrequired, argv[0]);
              return EXIT_FAILURE;
            }
        }
      else if (argv[1][0] == '-')
        {
          /* Unknown option */

          nsh_error(vtbl, g_fmtsyntax, argv[0]);
          return EXIT_FAILURE;
        }
      else
        {
#ifndef CONFIG_NSH_DISABLESCRIPT
          /* Execute the shell script */

          return nsh_script(vtbl, argv[0], argv[1]);
#else
          return EXIT_FAILURE;
#endif
        }
    }

  /* Then enter the command line parsing loop */

  for (; ; )
    {
      /* For the case of debugging the USB console...
       * dump collected USB trace data
       */

#ifdef CONFIG_NSH_USBDEV_TRACE
      nsh_usbtrace();
#endif

      /* Get the next line of input. readline() returns EOF
       * on end-of-file or any read failure.
       */

#ifdef CONFIG_NSH_CLE
      /* cle() normally returns the number of characters read, but will
       * return a negated errno value on end of file or if an error
       * occurs. Either  will cause the session to terminate.
       */

      ret = cle_fd(pstate->cn_line, g_nshprompt, CONFIG_NSH_LINELEN,
           infd, OUTFD(pstate));
      if (ret < 0)
        {
          fprintf(pstate->cn_errstream, g_fmtcmdfailed, "nsh_session",
                  "cle", NSH_ERRNO_OF(-ret));
          continue;
        }
#else
      /* Display the prompt string */

      nshprogress('P');
#ifdef CONFIG_ARCH_CHIP_RP2040
      write(OUTFD(pstate), "nsh> ", 5);
#else
      write(OUTFD(pstate), g_nshprompt, strlen(g_nshprompt));
#endif
      nshprogress('p');

      /* readline() normally returns the number of characters read, but
       * will return EOF on end of file or if an error occurs.  EOF
       * will cause the session to terminate.
       */

      nshprogress('L');
      nshprogress('R');  /* Before readline */
      ret = readline_fd(pstate->cn_line, CONFIG_NSH_LINELEN,
        infd, OUTFD(pstate));
      nshprogress('l');  /* After readline returns */

      if (ret > 0)
        {
          nshprogress('S');  /* Readline success */
          /* Command line bytes are available and we are about to hand off
           * to parser/dispatcher.
           */
        }

      if (ret == EOF)
        {
          nshprogress('E');

#ifdef CONFIG_ARCH_CHIP_RP2040
          {
            int errcode = errno;
            int cidx;
            int candidates[6];
            int candidate_count = 0;

            /* On this RP2040 branch, stdin descriptors can be late/unstable.
             * Do not terminate the shell on transient EOF-like conditions;
             * keep probing for any usable input descriptor and retry.
             * BUT: limit retries to prevent infinite loops.
             */

            eof_retry_count++;
            if (eof_retry_count > 1000)
              {
                /* Keep session alive and back off instead of exiting on RP2040.
                 * Exiting here causes the observed X8 loop without recovery.
                 */
                nshprogress('X');
                eof_retry_count = 0;
                usleep(50000);
                continue;
              }

            if (errcode == 0 || errcode == EAGAIN || errcode == EWOULDBLOCK ||
                errcode == EBADF || errcode == ENOTTY || errcode == EIO)
              {
                int found_fd = -1;

                /* Probe only a bounded set of likely descriptors. */
                candidates[candidate_count++] = infd;

                if (INFD(pstate) != infd)
                  {
                    candidates[candidate_count++] = INFD(pstate);
                  }

                if (OUTFD(pstate) != infd && OUTFD(pstate) != INFD(pstate))
                  {
                    candidates[candidate_count++] = OUTFD(pstate);
                  }

                if (ERRFD(pstate) != infd && ERRFD(pstate) != INFD(pstate) &&
                    ERRFD(pstate) != OUTFD(pstate))
                  {
                    candidates[candidate_count++] = ERRFD(pstate);
                  }

                if (0 != infd && 0 != INFD(pstate) && 0 != OUTFD(pstate) && 0 != ERRFD(pstate))
                  {
                    candidates[candidate_count++] = 0;
                  }

                for (cidx = 0; cidx < candidate_count; cidx++)
                  {
                    int probe = candidates[cidx];
                    if (probe < 0)
                      {
                        continue;
                      }

                    if (nsh_rp2040_fd_usable_input(probe))
                      {
                        found_fd = probe;
                        if (probe != infd)
                          {
                            infd = probe;
                            nshprogress('R');
                            eof_retry_count = 0;
                          }

                        break;
                      }
                  }

                if (found_fd < 0)
                  {
                    usleep(20000);
                  }
                else
                  {
                    /* Even if fd found, add yield to prevent busy-loop */
                    usleep(5000);
                  }

                continue;
              }
          }
#endif

          /* NOTE: readline() does not set the errno variable, but
           * perhaps we will be lucky and it will still be valid.
           */

          fprintf(pstate->cn_errstream, g_fmtcmdfailed, "nsh_session",
                  "readline", NSH_ERRNO);
          ret = EXIT_SUCCESS;
          break;
        }
#endif

      /* Parse process the command */

      nshprogress('P');  /* Before nsh_parse call */
      nsh_parse(vtbl, pstate->cn_line);
      nshprogress('p');  /* After nsh_parse call */
      fflush(pstate->cn_outstream);

    #ifdef CONFIG_ARCH_CHIP_RP2040
      /* RP2040 dual-core recovery fence: after command dispatch, drain stale
       * RX bytes accumulated during command execution and yield briefly before
       * re-entering readline().
       */
          {
            struct pollfd pfd;
            char dumpbuf[32];

            pfd.fd = infd;
            pfd.events = POLLIN;

            while (poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN))
              {
                if (read(infd, dumpbuf, sizeof(dumpbuf)) <= 0)
                  {
                    break;
                  }
              }
          }

                syslog(LOG_INFO, "NSH:drain done\n");

      usleep(5000);
    #endif
    }

  return ret;
}
