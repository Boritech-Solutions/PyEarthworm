
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: time_ew.c 3669 2009-06-19 16:50:18Z stefan $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.13  2009/06/19 16:50:18  stefan
 *     formatting change only
 *
 *     Revision 1.12  2009/06/18 18:42:30  stefan
 *     fixed gettimeofday by using Microsoft's suggested method in Remarks section here: http://msdn.microsoft.com/en-us/library/ms724928(VS.85).aspx
 *
 *     Revision 1.11  2009/06/15 19:11:02  paulf
 *     fixed windows specific include
 *
 *     Revision 1.10  2009/06/15 18:42:05  paulf
 *     added in gettimeofday() for windows
 *
 *     Revision 1.9  2005/08/18 05:00:00  davidk
 *     Fixed bug in timegm_ew() that matched the previous bug fixed in utc_ctime_ew().
 *
 *     Revision 1.8  2005/08/01 16:06:30  davidk
 *     Fixed bug in utc_time_ew(), where the wrong buffersize was passed to strncpy()
 *     and strange things were occuring even if the buffer passed was sufficently small.
 *
 *     Revision 1.7  2004/07/13 04:27:31  davidk
 *     Fixed bug, where _tzset() was not called at the end of timegm_ew() and
 *     utc_ctime_ew(), in order to properly restore the pre-call timezone settings.
 *
 *     Revision 1.6  2004/07/13 01:14:02  davidk
 *     Fixed bug in timegm_ew() so that it consistently produces a UTC based
 *     value.  The prior version required tzset() to be called prior to the call, and
 *     several moons to be aligned.
 *     Added function utc_ctime_ew(), to print a time_t value as a UTC ascii time.
 *
 *     Revision 1.5  2001/01/23 16:49:43  dietz
 *     Corrected roundoff problem in datestr23 and datestr23_local
 *
 *     Revision 1.4  2000/11/30 22:12:07  lombard
 *     fixed bug in timegm_ew: _timezone variable was not being set
 *     by call to _tzset() before call to mktime(). And the changes
 *     to the TZ environment variable were unnecessary.
 *
 *     Revision 1.3  2000/09/14 19:26:23  lucky
 *     Added datestr23_local which returns time string in local time
 *
 *     Revision 1.2  2000/03/10 23:35:58  davidk
 *     added includes from stdlib.h and stdio.h to resolve some compiler warnings.
 *
 *     Revision 1.1  2000/02/14 18:53:30  lucky
 *     Initial revision
 *
 *
 */


     /********************************************************
      *              time_ew.c   Windows NT version          *
      *                                                      *
      *  This file contains earthworm multi-thread safe      *
      *  versions of time routines.                          *
      ********************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys\timeb.h>
#include <sys\types.h>
#include <time_ew.h>
#include <windows.h>

/********************************************************
 *  gmtime_ew() converts time in seconds since 1970 to  *
 *  a time/date structure expressed as UTC (GMT)        *
 ********************************************************/
struct tm *gmtime_ew( const time_t *epochsec, struct tm *res )
{
    *res = *gmtime( epochsec );
    return( res );
}

/********************************************************
 *  localtime_ew() converts time in seconds since 1970  *
 *  to a time/date structure expressed as local time    *
 *  (using time zone and daylight savings corrections)  *
 ********************************************************/
struct tm *localtime_ew( const time_t *epochsec, struct tm *res )
{
    *res = *localtime( epochsec );
    return( res );
}

/********************************************************
 *  ctime_ew() converts time in seconds since 1970 to   *
 *  a 26 character string expressed as local time       *
 *  (using time zone and daylight savings corrections)  *
 *   Example:  "Fri Sep 13 00:00:00 1986\n\0"           *
 ********************************************************/
char *ctime_ew( const time_t *epochsec, char *buf, int buflen )
{
    strcpy( buf, ctime( epochsec ) );
    return( buf );
}

/********************************************************
 *  asctime_ew() converts time/date structure to        *
 *  a 26 character string                               *
 *   Example:  "Fri Sep 13 00:00:00 1986\n\0"           *
 ********************************************************/
char *asctime_ew( const struct tm *tm, char *buf, int buflen )
{
    strcpy( buf, asctime( tm ) );
    return( buf );
}

/*******************************************************
 * hrtime_ew() returns a high-resolution system clock  *
 *             time as a double in seconds since       *
 *             midnight Jan 1, 1970                    *
 *******************************************************/
double hrtime_ew( double *tnow )
{
    struct _timeb t;

    _ftime( &t );
    *tnow = (double)t.time + (double)t.millitm*0.001;
    return( *tnow );
}

/********************************************************
 *                      timegm_ew()                     *
 * Convert struct tm to time_t using GMT as time zone   *
 ********************************************************/
time_t timegm_ew( struct tm *stm )
{
   time_t tt;

   char szTZExisting[32] = "TZ=";
   char * pTZ;

   /*  There are several ways to set the timezone in Windows.
       The easisest is by using the TZ environment variable.
       Others include directly setting the _timezone and _daylight
       global variables, but you must be careful that mktime/localtime
       does not call _tzset() and change the values out from under you.
       DK 071204
     *******************************************************************/


   /* 1:  Retrieve the current TZ environment variable if set */
   pTZ = getenv("TZ");

   /* 2:  If the current TZ environment var is valid, then save it. */
   if(pTZ)
   {
     /* remember we have "TZ=" stored as the first 3 chars of the array */
     strncpy(&szTZExisting[3], pTZ, sizeof(szTZExisting) - 4);
     szTZExisting[sizeof(szTZExisting)-1] = 0x00;
   }
   /*  else - defaults to a null string */

   /* 3:  Set the TZ var to UTC */
   _putenv("TZ=UTC");

   /* 3.5:  Set the globals, in case someone has futzed with them, and
      through some codepath _tzset() chooses not to change them   */
   _timezone = 0;  // set the timezone-offset global var to 0 
   _daylight = 0;  // set the DST global var to 0 

   /* 4:  Call _tzset() to ensure the time-related global vars are set properly */
   _tzset();

   /* 5:  Perform the desired UTC/GMT time operation */
   tt = mktime( stm ); 

   /* 6:  Change the TZ var back */
   _putenv(szTZExisting);

   /* 7:  Call _tzset() to reset the globals */
   _tzset();


   /*************************************************
    alternative method:
      Based on review/testing of the existing CRT code, the following should
      also work, but is definitely more back-door/hackish.
      It is much more efficient though.
      DK 071204

   _tzset();   // ensure _tzset() has been called, so that CRT doesn't automatically
               // call it again.
   _timezone = 0;  // set the timezone-offset global var to 0 
   _daylight = 0;  // set the DST global var to 0 

   tt = mktime( stm ); 

   _tzset();    // call tzset() to restore the global vars to their proper values
 
   *************************************************/

   return( tt );
}


/**********************************************************
 * Converts time (double, seconds since 1970:01:01) to    *
 * a 22-character, null-terminated string in the form of  *
 *            yyyy/mm/dd hh:mm:ss.ss                      *
 * Time string returned is in UTC time                    *
 * Target buffer must be 23-chars long to have room for   *
 * null-character                                         *
 **********************************************************/ 
char *datestr23( double t, char *pbuf, int len )
{  
   time_t    tt;       /* time as time_t                  */
   struct tm stm;      /* time as struct tm               */
   int       t_hsec;   /* hundredths-seconds part of time */

/* Make sure target is big enough
 ********************************/
   if( len < DATESTR23 ) return( (char *)NULL );

/* Convert double time to other formats 
 **************************************/
   t += 0.005;  /* prepare to round to the nearest 100th */
   tt     = (time_t) t;
   t_hsec = (int)( (t - tt) * 100. );
   gmtime_ew( &tt, &stm );

/* Build character string
 ************************/
   sprintf( pbuf, 
           "%04d/%02d/%02d %02d:%02d:%02d.%02d",
            stm.tm_year+1900,
            stm.tm_mon+1,
            stm.tm_mday,
            stm.tm_hour,
            stm.tm_min,
            stm.tm_sec,            
            t_hsec );
 
   return( pbuf );
}


/**********************************************************
 * Converts time (double, seconds since 1970:01:01) to    *
 * a 22-character, null-terminated string in the form of  *
 *            yyyy/mm/dd hh:mm:ss.ss                      *
 * Time string returned is in LOCAL time                  *
 * Target buffer must be 23-chars long to have room for   *
 * null-character                                         *
 **********************************************************/ 
char *datestr23_local( double t, char *pbuf, int len )
{  
   time_t    tt;       /* time as time_t                  */
   struct tm stm;      /* time as struct tm               */
   int       t_hsec;   /* hundredths-seconds part of time */

/* Make sure target is big enough
 ********************************/
   if( len < DATESTR23 ) return( (char *)NULL );

/* Convert double time to other formats 
 **************************************/
   t += 0.005;  /* prepare to round to the nearest 100th */
   tt     = (time_t) t;
   t_hsec = (int)( (t - tt) * 100. );
   localtime_ew( &tt, &stm );

/* Build character string
 ************************/
   sprintf( pbuf, 
           "%04d/%02d/%02d %02d:%02d:%02d.%02d",
            stm.tm_year+1900,
            stm.tm_mon+1,
            stm.tm_mday,
            stm.tm_hour,
            stm.tm_min,
            stm.tm_sec,            
            t_hsec );
 
   return( pbuf );
}



/********************************************************
 *                      utc_ctime_ew()                  *
 * Print out a time_t value as a UTC date/time          *
 * regardless of the system timezone.                   *
 * same as ctime() but uses UTC to interpret time_t     *
 * value.    - davidk 07/12/04                          *
 * The code in this function was extracted from         *
 * timegm_ew()                                          *
 ********************************************************/
char * utc_ctime_ew(time_t * pTime)
{

   char szTZExisting[32] = "TZ=";
   char * szTime;
   char * pTZ;

   /*  There are several ways to set the timezone in Windows.
       The easisest is by using the TZ environment variable.
       Others include directly setting the _timezone and _daylight
       global variables, but you must be careful that mktime/localtime
       does not call _tzset() and change the values out from under you.
       DK 071204
     *******************************************************************/

   /* 1:  Retrieve the current TZ environment variable if set */
   pTZ = getenv("TZ");

   /* 2:  If the current TZ environment var is valid, then save it. */
   if(pTZ)
   {
     /* remember we have "TZ=" stored as the first 3 chars of the array */
     strncpy(&szTZExisting[3], pTZ, sizeof(szTZExisting) - 4);
     szTZExisting[sizeof(szTZExisting)-1] = 0x00;
   }
   /*  else - defaults to a null string */

   /* 3:  Set the TZ var to UTC */
   _putenv("TZ=UTC");

   /* 3.5:  Set the globals, in case someone has futzed with them, and
      through some codepath _tzset() chooses not to change them   */
   _timezone = 0;  // set the timezone-offset global var to 0 
   _daylight = 0;  // set the DST global var to 0 

   /* 4:  Call _tzset() to ensure the time-related global vars are set properly */
   _tzset();

   /* 5:  Perform the desired UTC/GMT time operation */
   szTime = ctime(pTime); 

   /* 6:  Change the TZ var back */
   _putenv(szTZExisting);

   /* 7:  Call _tzset() to reset the globals */
   _tzset();

   return(szTime);
}

/*
 * http://social.msdn.microsoft.com/forums/en-US/vcgeneral/thread/430449b3-f6dd-4e18-84de-eebd26a8d668
 */
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME ft, sysFileTime;
    ULARGE_INTEGER tmpres;
    ULARGE_INTEGER tmpdelta;

	SYSTEMTIME st;
    static int tzflag;

    if (NULL != tv)
    {
        GetSystemTimeAsFileTime(&ft);
		
		/* Initialize to the first second of 1970 */
		/* st.wDayOfWeek = 1; */
		st.wDay = 1;
		st.wHour = 0;
		st.wHour = 0;
		st.wMilliseconds = 0;
		st.wMinute = 0;
		st.wMonth = 1;
		st.wSecond = 0;
		st.wYear = 1970;

		SystemTimeToFileTime(&st, &sysFileTime);

		tmpdelta.HighPart = sysFileTime.dwHighDateTime;
		tmpdelta.LowPart = sysFileTime.dwLowDateTime;
				
		tmpres.HighPart = ft.dwHighDateTime;
		tmpres.LowPart = ft.dwLowDateTime;

        /* converting file time to unix epoch
         * tmpres -= DELTA_EPOCH_IN_MICROSECS; 
		 * 116444736000000000 is this value not 11644473600000000 as published here:
		 * http://social.msdn.microsoft.com/forums/en-US/vcgeneral/thread/430449b3-f6dd-4e18-84de-eebd26a8d668 */
		tmpres.QuadPart -= tmpdelta.QuadPart;
        tmpres.QuadPart /= 10;  /*convert into microseconds*/
        tv->tv_sec = (long)(tmpres.QuadPart / 1000000UL);
        tv->tv_usec = (long)(tmpres.QuadPart % 1000000UL);
    }

    if (NULL != tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;

    }

    return 0;
}

