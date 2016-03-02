/*
 * sleepenh.c - enhanced sleep command
 *
 * Copyright (C) 2003 Pedro Zorzenon Neto
 * Copyright (C) 2014 Nicolas Schier <nicolas+debian@hjem.rpa.no>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
 *
 * USAGE INSTRUCTIONS
 *
 * this program is an enhanced version of sleep, you can use as
 * a simple sleep program, with microseconds
 * Example: (sleeps 45.123234 seconds)
 *    sleepenh 45.123234
 *
 * if it is called with two arguments, it acts as a sleep program
 * that can be called in sequence and will not have cumulative
 * errors from one call to another.
 * Example: (time from one 'ls' to other is 12.4 seconds)
 *   VAL=`sleepenh 0` ; while true; do ls; VAL=`sleepenh 12.4`; done
 *
 * Exit status:
 * 0  - success. I needed to sleep sometime
 * 1  - success. I did not need to sleep
 * >9 - failure.
 * 10 - failure. not enough command line arguments
 * 11 - failure. did not receive sigalrm
 * 12 - failure. argument is not a finite number
 * 13 - failure. could not get current time (gettimeofday)
 *
 * shell script usage example: see manpage
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#define SHORTEST_SLEEP 0.00001 /* 10msec, a timeslice */

static int sigflag=0;

void got_signal() {
  sigflag=1;
}

void version(FILE *f)
{
	fprintf(f,
		"sleepenh " VCSVERSION "\n"
		"\n"
		"Copyright (C) 2003 Pedro Zorzenon Neto\n"
		"Copyright (C) 2014 Nicolas Schier\n"
		"License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>.\n"
		"This is free software: you are free to change and redistribute it.\n"
		"There is NO WARRANTY, to the extent permitted by law.\n");
}

void usage(FILE *f)
{
	fprintf(f,
		"Usage: %s [[--warp|-w] INITIALTIME] TIMETOSLEEP\n"
		"\n"
		"An enhanced sleep program.\n"
		"\n"
		"Options:\n"
		"  -h, --help     display this help and exit\n"
		"  -w, --warp   warp resulting timestamp, when there is no need\n"
		"               to sleep.  An immediatly following call of\n"
		"               sleepenh with the resulting TIMESTAMP would\n"
		"               most probably result in a real sleep.\n"
		"  -V, --version  output version information and exit\n"
		"\n"
		"TIMETOSLEEP is in seconds, microsecond resolution, ex: 80.123456.\n"
		"INITIALTIME is the output value of a previous execution of sleepenh.\n",
		program_invocation_short_name);
}

int main(int argc, char *argv[]) {
  struct timeval tv;
  struct timezone tz;
  struct itimerval itv;
  struct sigaction sigact;
  double st;  /* sleep time */
  double et;  /* end time */
  double now; /* now */
  double it;  /* initial time */
  double sleep_time; /* effective time to sleep */
  int warp = 0;

  if ((argc > 1) &&
      (!strcmp(argv[1], "--warp") || !strcmp(argv[1], "-w"))) {
		warp = 1;
		argc--, argv++;
  }

  if (argc==1)
    {
      version(stderr);
      fprintf(stderr, "\n");
      usage(stderr);
      return 10; /* failure, bad arguments */
    }

  if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h") ||
      !strcmp(argv[1], "--usage") || !strcmp(argv[1], "-u")) {
	  usage(stdout);
	  return 0;
  }

  if (!strcmp(argv[1], "--version") || !strcmp(argv[1], "-V")) {
	  version(stdout);
	  return 0;
  }

  if(gettimeofday(&tv,&tz)!=0)
    {
      return 13; /* failure, could not get current time */
    }

  st=strtod(argv[argc-1],NULL);
  if(finite(st)==0)
    {
      return 12; /* failure, argument is not a finite number */
    }
  now=tv.tv_sec+(tv.tv_usec*0.000001);

  if (argc==2)
    {
      /* without initialtime */
      it=now;
    }
  else
    {
      /* with initialtime */
      it=strtod(argv[1],NULL);
      if(finite(it)==0)
	{
	  return 12; /* failure, argument is not a finite number */
	}
    }

  et=it+st;
  sleep_time = et - now;

  if (sleep_time < SHORTEST_SLEEP)
    {
      if (warp) {
		/* warp in time -> loose events, but keep event regularity */
		int tmp;

		tmp = (now - it) / st;
		et = it + tmp * st;
      }
      /* has already timed out, shorted than a timeslice */
      printf("%f\n",et);
      return 1; /* success, time out */
    }

  /* set signal handler */
  memset(&sigact, 0, sizeof(sigact));
  sigact.sa_handler=&got_signal;
  sigaction (SIGALRM, &sigact, NULL);

  /* set timer */
  itv.it_value.tv_sec=(long int) sleep_time;
  itv.it_value.tv_usec=((long int) (sleep_time*1000000)) % 1000000;
  itv.it_interval.tv_sec=0;
  itv.it_interval.tv_usec=0;
  setitimer(ITIMER_REAL,&itv,NULL);

  pause(); /* wait for signal */

  printf("%f\n",et);

  if (sigflag==1)
    {
      return 0; /* success */
    }

  return 11; /* failure */
}
