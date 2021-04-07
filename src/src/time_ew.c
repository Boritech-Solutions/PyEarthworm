     /********************************************************
      *              time_ew.c   UNIX version                *
      *                                                      *
      *  This file contains earthworm multi-thread safe      *
      *  versions of time routines                           *
      *  Needs to be linked with -lposix4                    *
      ********************************************************/

#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "time_ew.h"

/********************************************************
 *  gmtime_ew() converts time in seconds since 1970 to  *
 *  a time/date structure expressed as UTC (GMT)        *
 ********************************************************/
struct tm *gmtime_ew( const time_t *epochsec, struct tm *res )
{
    gmtime_r( epochsec, res );
    return( res );
}

/********************************************************
 *                      timegm_ew()                     *
 * Convert struct tm to time_t using GMT as time zone   *
 ********************************************************/
#define MAXENV 128         
char    envTZ[MAXENV];  /* Space where environment variable TZ will be */
                        /* stored after the first call to timegm_ew()  */
         
time_t timegm_ew( struct tm *stm )
{
   char  *tz;
   time_t tt;
   char   TZorig[MAXENV];

/* Save current TZ setting locally
 *********************************/
   tz = getenv("TZ");
   if( tz != (char *) NULL )
   {
      if( strlen(tz) > MAXENV-4 ) 
      {
         printf("timegm_ew: unable to store current TZ environment variable.\n");
         return( -1 );
      }
   }
   sprintf( TZorig, "TZ=%s", tz );
  
/* Change time zone to GMT; do conversion
 ****************************************/
   sprintf( envTZ, "TZ=GMT" );
   if( putenv( envTZ ) != 0 )  
   {
      printf("timegm_ew: putenv: unable to set TZ environment variable.\n" );
      return( -1 );
   }
   tt = mktime( stm ); 

/* Restore original TZ setting
 *****************************/
   sprintf( envTZ, "%s", TZorig );
   if( putenv( envTZ ) != 0 )  
   {
     printf("timegm_ew: putenv: unable to restore TZ environment variable.\n" );
   }
   tzset();

   return( tt );
}

/********************************************************
 *  localtime_ew() converts time in seconds since 1970  *
 *  to a time/date structure expressed as local time    *
 *  (using time zone and daylight savings corrections)  *
 ********************************************************/
struct tm *localtime_ew( const time_t *epochsec, struct tm *res )
{
    localtime_r( epochsec, res );
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
    char *rc;

#ifdef __sgi
    char tbuf[32];	/* only need 26 */
    if ( (rc = ctime_r(epochsec, tbuf)) == (char *)NULL )
	return( rc );
    *buf = '\0';
    strncpy(buf,tbuf,buflen);
    if ( strlen(tbuf) >= buflen )
	buf[buflen-1] = '\0';
#else
#ifdef _UNIX
    char tbuf[32];	/* only need 26 */
    if ( (rc = ctime_r(epochsec, tbuf)) == (char *)NULL )
	return( rc );
    *buf = '\0';
    strncpy(buf,tbuf,buflen);
    if ( strlen(tbuf) >= buflen )
	buf[buflen-1] = '\0';
#else
    if ( (rc = ctime_r(epochsec, buf, buflen)) == (char *)NULL )
    	return( rc );
#endif
#endif

    return( buf );
}

/********************************************************
 *  asctime_ew() converts time/date structure to        *
 *  a 26 character string                               *
 *   Example:  "Fri Sep 13 00:00:00 1986\n\0"           *
 ********************************************************/
#define TBUF_SIZE_ASC 32
char *asctime_ew( const struct tm *tm, char *buf, int buflen )
{
    char tbuf[TBUF_SIZE_ASC];      /* only need 26 */

#ifdef _SOLARIS
    if ( asctime_r(tm, tbuf, TBUF_SIZE_ASC) == NULL )
#else
    if ( asctime_r(tm, tbuf) == NULL )
#endif
        return NULL;
    strncpy(buf, tbuf, buflen);
    buf[buflen-1] = '\0';
    return buf;
}

/*******************************************************
 * hrtime_ew() returns a high-resolution system clock  *
 *             time as a double                        *
 *******************************************************/
double hrtime_ew( double *tnow )
{
#ifndef _UNIX
    struct timespec t;

    if( clock_gettime( CLOCK_REALTIME, &t ) == 0 ) {
       *tnow = (double) t.tv_sec + (double)t.tv_nsec*0.000000001;
    }
    else {   
       *tnow = 0;
    }
#else
    struct timeval tv;

    gettimeofday(&tv, NULL);
    *tnow= tv.tv_sec+tv.tv_usec/1000000.;
#endif
    return( *tnow );
}

/**********************************************************
 * Converts time (double, seconds since 1970:01:01) to    *
 * a 22-character, null-terminated string in the form of  *
 *            yyyy/mm/dd hh:mm:ss.ss                      *
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
   t_hsec = (int)( (t - (double) tt) * 100. );
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
 * Time is displayed in LOCAL time                        *
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

/**********************************************************
 * Converts time (double, seconds since 1970:01:01) to   a null-terminated
 *string.
 *
 * @param t the epoch time in seconds                     *
 * @param pbuf pointer to the output buffer
 * @param len the maximum number of characters (20 - 38 depending on fractional
 *precision of seconds)
 * @param datesep the date separator character
 * this is displayed between date fields
 * @param datetimesep the date time separator character
 * this is displayed between the date and time fields
 **********************************************************/
char *datestrn(double t, char *pbuf, int len, char datesep, char datetimesep)
{
  time_t tt;     /* time as time_t                  */
  struct tm stm; /* time as struct tm               */
  double t_sec;  /* sec part of time */
  
  /* Convert double time to other formats
   **************************************/
  tt = (time_t)t;
  gmtime_ew(&tt, &stm);
  t_sec = t - tt + stm.tm_sec;
  
  /* Build character string
   ************************/
  snprintf(pbuf, len, "%04d%c%02d%c%02d%c%02d:%02d:%020.17f", stm.tm_year + 1900,
           datesep, stm.tm_mon + 1, datesep, stm.tm_mday, datetimesep,
           stm.tm_hour, stm.tm_min, t_sec);
  
  return (pbuf);
}
