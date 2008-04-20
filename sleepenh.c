/*
 * sleepenh.c - enhanced sleep command
 *
 * Copyright (C) 2003 - Pedro Zorzenon Neto
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
#define RCSID    "$Id: sleepenh.c,v 1.2 2003/02/24 17:17:48 pzn Exp $"
#define RCSREV   "$Revision: 1.2 $"
#define RCSDATE  "$Date: 2003/02/24 17:17:48 $"

static int sigflag=0;

void got_signal() {
  sigflag=1;
}

int main(int argc, char *argv[]) {
  struct timeval tv;
  struct timezone tz;
  struct itimerval itv;
  struct sigaction sigact;
  double st;  /* start time */
  double et;  /* end time */
  double now; /* now */
  double it;  /* interval */

  if (argc==1)
    {
      fprintf(stderr,
	      "sleepenh -- an enhanced sleep program.\n"
	      "         -- " RCSREV "\n"
	      "         -- " RCSDATE "\n"
	      "\n"
	      "Copyright (C) 2003 - Pedro Zorzenon Neto\n"
	      "Distributed under the conditions of FSF/GPL2 License.\n"
	      "See the source code for more copyright and license information.\n"
	      "\n"
	      "Usage: %s timetosleep\n"
	      "   or: %s initialtime timetosleep\n"
	      "\n"
	      "timetosleep is in seconds, microsecond resolution. Ex: 80.123456\n"
	      "initialtime is the output value of a previous execution of sleepenh.\n"
	      , argv[0], argv[0]);
      return 10; /* failure, bad arguments */
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
  it=et-now;

  if ( it < SHORTEST_SLEEP )
    {
      /* has already timed out, shorted than a timeslice */
      printf("%f\n",et);
      return 1; /* success, time out */
    }

  /* set signal handler */
  memset(&sigact, 0, sizeof(sigact));
  sigact.sa_handler=&got_signal;
  sigaction (SIGALRM, &sigact, NULL);

  /* set timer */
  itv.it_value.tv_sec=(long int) it;
  itv.it_value.tv_usec=((long int) (it*1000000)) % 1000000;
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
