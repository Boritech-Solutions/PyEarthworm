
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id$
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2010/01/02 00:39:51  kress
 *     bookeeping on gcf2ew.  reboot_mss and reboot_mss_ew now both work in linux.
 *
 *     Revision 1.1  2005/07/15 18:20:21  friberg
 *     Unix version of libsrc for POSIX systems
 *
 *     Revision 1.3  2004/03/22 21:07:22  kohler
 *     Added: #include <string.h>
 *     Prototype for strerror()
 *
 *     Revision 1.2  2000/07/13 16:39:45  lombard
 *     Added checks for error return from nanosleep()
 *
 *     Revision 1.1  2000/02/14 18:46:17  lucky
 *     Initial revision
 *
 *
 */

/********************************************************************
 *                 sleep_ew.c    for   SOLARIS                      *
 *                                                                  *
 *  Any program that uses this function must contain:		    *
 *   #include <earthworm.h> 					    *
 *  and must link with the posix4 library:			    *
 *   cc [ flag ... ] file ... -lposix4 [ library ... ]		    *
 ********************************************************************/

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/********************* sleep_ew for SOLARIS ******************
                    sleep alarmtime milliseconds
 **************************************************************/
void sleep_ew( unsigned alarmtime )
{
   struct timespec sleeptime;
   struct timespec timeleft;
   int err;

   sleeptime.tv_sec = (time_t) alarmtime / 1000;
   sleeptime.tv_nsec = (long) (1000000 * (alarmtime % 1000));

   while( nanosleep(&sleeptime, &timeleft) != 0 )
   {
      if ( (err = errno) == EINTR)
      {
        /*printf( "sleep_ew: interrupted by signal;" );*//*DEBUG*/ 
        /*printf( " %d msec left\n",
	        (int) (timeleft.tv_sec*1000 + timeleft.tv_nsec/1000000) );*//*DEBUG*/
	sleeptime = timeleft;
      }
      else
      {
         fprintf(stderr, "sleep_ew: error from nanosleep: %s\n",
                 strerror(err));
         fprintf(stderr,"\ttime requested = %.3f sec, time left = %.3f sec\n",
                 sleeptime.tv_sec + sleeptime.tv_nsec*1.e-9,
                 timeleft.tv_sec + timeleft.tv_nsec*1.e-9);
       }
   }

   return;
}
