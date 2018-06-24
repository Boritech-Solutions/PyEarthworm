
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: kom.h 4751 2012-04-04 20:46:28Z scott $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

/*
 *   kom.h:  include file for kom.c
 */

#ifndef KOM_H
#define KOM_H

/* Prototypes for functions in kom.c
 ***********************************/
int    k_open( char * );        /* open new file for k-system input     */
int    k_open_as_ini( char * );	/* k_open w/ file in .ini format        */
void   k_treat_as_ini( void );	/* switch to .ini format                */
void   k_treat_as_ew( void );	/* switch to ew format                  */
int    k_close( void );         /* close current file                   */
char  *k_get( void );           /* return pointer to current command    */
void   k_dump( void );          /* print last card read from file       */
int    k_err( void );           /* return last error code and clear     */
int    k_put( char * );         /* insert command line to be parsed     */
int    k_rd( void );            /* read a line from file into buffer    */
int    k_rd_raw( int flag_expansion_level );
                                /* read a line from file into buffer
				 * and expand variables depending on
				 * value of flag_expansion_level        */
int    k_its( char * );         /* compare string of last token to      */
                                /* given string; 1=match 0=no match     */
char  *k_com( void );           /* returns last line read from file     */
char  *k_str( void );           /* return next token as pntr to string  */
double k_val( void );           /* return next token as a double real   */
int    k_int( void );           /* return next token as an integer      */
long   k_long( void );          /* return next token as a long integer  */

/* Change function for logging messages, default is fprintf to stderr */
void set_kom_log(void (*par_kom_log_func)(char *) );

/*
 * It is possible to enable variable expansion over tree levels:
 *    1) No expansion at all
 *    2) Variable expansion only from environment
 *    3) Variable expansion from environment and file
 *
 * Following files must always be excluded from variable expansion:
 *    earthworm.d
 *    earthworm_global.d
 *    earthworm_commonvars.d
 *
 */
#define KOM_NO_EXPANSION        0
#define KOM_EXPANSION_FROM_ENV  1
#define KOM_EXPANSION_FROM_FILE 2


#endif

