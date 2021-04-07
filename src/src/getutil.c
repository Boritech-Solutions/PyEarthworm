/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: getutil.c 7116 2018-02-14 22:27:54Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.7  2009/12/14 19:21:39  scott
 *     Added threading & recovery from bad reconfigure to startstop
 *
 *     Revision 1.6  2007/03/28 20:53:27  paulf
 *     added MACOSX flag for using / instead of \
 *
 *     Revision 1.5  2006/03/10 13:49:50  paulf
 *     minor linux related fixes to removing _SOLARIS from the include line
 *
 *     Revision 1.4  2004/07/29 17:51:28  dietz
 *     Added functions GetKeyName,GetInstName,GetModIdName,GetTypeName,
 *     GetLocalInstName which return a pointer to a character string
 *     set in the earthworm*d files.
 *
 *     Revision 1.3  2000/07/27 16:23:31  lucky
 *     Implemented global limits, from earthworm.h, in the sizes of installation ids,
 *     message types, ring names, and module names.
 *
 *     Revision 1.2  2000/07/08 19:49:11  lombard
 *     fprintf statment had extra %s format
 *
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */

/*
 *  getutil.c  functions for looking up shared memory rings,
 *             installations, modules, and message types in
 *             Earthworm tables.  Given a character-string name,
 *             these functions return a numerical value.
 *
 *             The installation table is set up in the include-file
 *             earthworm.h; it is global to all Earthworm installations.
 *             Changes to this table require a recompilation of the
 *             whole Earthworm tree.
 *  
 *             The shared memory ring, module, and message type tables
 *             are set up in the ascii table file (whose name is stored 
 *             in the global variable, TableFile) which resides in the 
 *             EW_PARAMS directory.  Changes to this file do not require
 *             recompilation.
 * 
 *  990603:LDD Modified GetUtil_LoadTable to read in installation ids
 *             from an ascii file (instead of being defined in earthworm.h)
 *             so that installations can be added without recompiling!  LDD
 *             
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "earthworm.h"
#include "startstop_lib.h"
#include "kom.h"
#include "earthworm_complex_funcs.h"

/* Table of shared memory ring names & their keys
 ************************************************/
#define RINGLEN MAX_RING_STR
static struct {
  long   key;
  char   name[RINGLEN+1];
} EW_Ring[MAX_RING];
static int Max_Ring = 0;      /* # ring/keys currently in table */

/* Table of module names and their module_ids
 ********************************************/
#define MAXMODID 256
#define MODLEN  MAX_MOD_STR
static struct {
  unsigned char id;
  char          name[MODLEN+1];
} EW_Module[MAXMODID];
static int Max_ModuleId = 0;  /* # modules currently in table */

/* Table of message names and their type-values
 **********************************************/
#define MAXMSGTYPE 256 
#define MSGLEN 	   MAX_TYPE_STR 
static struct {
  unsigned char type;
  char          name[MSGLEN+1];
} EW_Message[MAXMSGTYPE];
static int Max_MessageType = 0;  /* # msg types currently in table */

/* Table of Installation names and their instids
 ***********************************************/
#define MAXINSTID 256
#define INSTLEN   MAX_INST_STR 
static struct {
  unsigned char id;
  char          name[INSTLEN+1];
} EW_Installation[MAXINSTID];
static int Max_Installation = 0;  /* # installations currently in table */

#define MAXLEN 255
static char  FullTablePath[MAXLEN+1];
static char *TableFile[] = {"earthworm_global.d","earthworm.d"}; 
static char  nTableFile  = 2;
static int   LoadTables  = 1;   /* Do tables need to be loaded from */
                                /* the TableFile?    1=yes 0=no     */
 

/* internal function for printing getutil errors, uses logit if initialized */

void getutil_logit(char *format, ... )
{
   auto va_list ap;

   va_start( ap, format );
   if (is_logit_initialized()) {
         logit_core("e", format, ap);
   } else {
	 vfprintf(stderr, format, ap);
   }
   va_end( ap );
}

         /*******************************************************
          *                    GetKeyCore                       *
          *                                                     *
          *  Convert ring name to key number using table        *
          *  in TableFile                                       *
          *  Return <key number> on success;                    *
          *     If the specified ring name is unknown,          *
          *        return -1 if useDefault is 0                 *
          *        return default otherwise                     *
          *******************************************************/
static long GetKeyCore( char *ringName, long defaultVal, char useDefault )
{
   int i;

   if( LoadTables ) {
       GetUtil_LoadTable();
       LoadTables = 0;
   }

/* Find transport ring name in earthworm.d table
 ***********************************************/
   for ( i = 0; i < Max_Ring; i++ )
      if ( strcmp( EW_Ring[i].name, ringName ) == 0 )
         break;

/* Didn't find ring name in table
 ********************************/
   if ( i == Max_Ring )
   {
      if ( !useDefault ) {
         getutil_logit("GetKey: Invalid ring name <%s>\n", ringName );
         return( -1 );
      } else 
         return defaultVal;
   }

/* Found it!
 ***********/
   return( EW_Ring[i].key );
}

         /*******************************************************
          *                      GetKey                         *
          *                                                     *
          *  Convert ring name to key number using table        *
          *  in TableFile                                       *
          *  Return <key number> on success;                    *
          *          -1 if the specified ring name is unknown.  *
          *******************************************************/
long GetKey( char *ringName )
{
   return GetKeyCore( ringName, 0, 0 );
}

         /*******************************************************
          *                GetKeyWithDefault                    *
          *                                                     *
          *  Convert ring name to key number using table        *
          *  in TableFile                                       *
          *  Return <key number> on success;                    *
          *       defaultVal if specified ring name is unknown. *
          *******************************************************/
long GetKeyWithDefault( char *ringName, long defaultVal )
{
	return GetKeyCore( ringName, defaultVal, 1 );
}


         /*******************************************************
          *                   GetKeyName                        *
          *                                                     *
          * Given the numeric memory key, return a pointer      *
          * to its character string name.  Returns NULL pointer *
          * if memory key is not defined in earthworm.d         *
          *******************************************************/
char *GetKeyName( long key )
{
   int i;

   if( LoadTables ) {
       GetUtil_LoadTable();
       LoadTables = 0;
   }

/* Find memory key in earthworm.d table
 **************************************/
   for ( i = 0; i <  Max_Ring; i++ )
      if ( key == EW_Ring[i].key ) return( EW_Ring[i].name );

/* Didn't find memory key in table
 *********************************/
   return( (char *)NULL );
}


         /*******************************************************
          *                   GetLocalInst                      *
          *                                                     *
          *  Convert the local installation name, set by the    *
          *  environment variable EW_INSTALLATION, to a number  *
          *  using the table in earthworm.h                     *
          *  Return  0 on success;                              *
          *         -1 if the environment variable is not       *
          *            defined;                                 *
          *         -2 if the env. variable has no value        *
          *         -3 if the specified installation name is    *
          *            not listed in earthworm.h                *
          *******************************************************/
int GetLocalInst( unsigned char *localId )
{
   char *envInst;

   *localId = 0;

/* Get the installation name from environment variable EW_INSTALLATION
   *******************************************************************/
   envInst = getenv( "EW_INSTALLATION" );

   if ( envInst == (char *) NULL )
   {
      getutil_logit( "GetLocalInst: Environment variable EW_INSTALLATION not defined.\n" );
      return( -1 );
   }

   if ( *envInst == '\0' )
   {
      getutil_logit( "GetLocalInst: Environment variable EW_INSTALLATION defined, but has no value.\n" );
      return( -2 );
   }

   if( LoadTables ) {
       GetUtil_LoadTable();
       LoadTables = 0;
   }

/* Find the id-value in the table
 ********************************/
   if ( GetInst( envInst, localId ) < 0 )
   {
      getutil_logit( "GetLocalInst: Environment variable EW_INSTALLATION has invalid value <%s>\n", envInst );
      return( -3 );
   }

   return( 0 );
}


         /*******************************************************
          *                 GetLocalInstName                    *
          *                                                     *
          * Returns a pointer to the environment variable       *
          * EW_INSTALLATION which sets the local installation   *
          * name.  Returns a NULL pointer if the environment    *
          * variable is not set.                                *
          *******************************************************/
char *GetLocalInstName( void )
{
   return( getenv( "EW_INSTALLATION" ) );
}


         /*******************************************************
          *                      GetInst                        *
          *                                                     *
          *  Convert installation name to number using table    *
          *  in earthworm_global.d                              *
          *  Return  0 on success;                              *
          *         -1 if the specified installation name is    *
          *            unknown.                                 *
          *******************************************************/
int GetInst( char *instName, unsigned char *instId )
{
   int i;

   if( LoadTables ) {
       GetUtil_LoadTable();
       LoadTables = 0;
   }

/* Find installation name in earthworm.d table
 *********************************************/
   for ( i = 0; i < Max_Installation; i++ )
      if ( strcmp( EW_Installation[i].name, instName ) == 0 )
         break;

/* Didn't find installation name in table
 ****************************************/
   if ( i == Max_Installation )
   {
      getutil_logit( "GetInst: Invalid installation name <%s>\n", instName );
     *instId = 0;
      return( -1 );
   }

/* Found it!
 ***********/
  *instId = EW_Installation[i].id;
   return( 0 );
}

         /*******************************************************
          *                   GetInstName                       *
          *                                                     *
          * Given the numeric Installation ID, return a pointer *
          * to its character string name.  Returns NULL pointer *
          * if InstID is not defined in earthworm_global.d      *
          *******************************************************/
char *GetInstName( unsigned char instid )
{
   int i;

   if( LoadTables ) {
       GetUtil_LoadTable();
       LoadTables = 0;
   }

/* Find installation id in earthworm_global.d table
 **************************************************/
   for ( i = 0; i < Max_Installation; i++ )
      if ( instid == EW_Installation[i].id ) return( EW_Installation[i].name );

/* Didn't find installation id in table
 **************************************/
   return( (char *)NULL );
}


         /*******************************************************
          *                      GetModId                       *
          *                                                     *
          *  Convert module name to modid number using table    *
          *  defined in TableFile                               *
          *  Return  0 on success;                              *
          *         -1 if the specified module name is unknown. *
          *******************************************************/
int GetModId( char *modName, unsigned char *modId )
{
   int i;

   if( LoadTables ) {
       GetUtil_LoadTable();
       LoadTables = 0;
   }

/* Find module name in earthworm.d table
 ***************************************/
   for ( i = 0; i < Max_ModuleId; i++ )
      if ( strcmp( EW_Module[i].name, modName ) == 0 )
         break;

/* Didn't find module name in table
 **********************************/
   if ( i == Max_ModuleId )
   {
      getutil_logit( "GetModId: Invalid module name <%s>\n", modName );
     *modId = 0;
      return( -1 );
   }

/* Found it!
 ***********/
  *modId = EW_Module[i].id;
   return( 0 );
}


         /*******************************************************
          *                   GetModIdName                      *
          *                                                     *
          * Given the numeric Module ID, return a pointer       *
          * to its character string name.  Returns NULL pointer *
          * if Module ID is not defined in earthworm.d          *
          *******************************************************/
char *GetModIdName( unsigned char modid )
{
   int i;

   if( LoadTables ) {
       GetUtil_LoadTable();
       LoadTables = 0;
   }

/* Find module_id in earthworm.d table
 *************************************/
   for ( i = 0; i < Max_ModuleId; i++ )
      if ( modid == EW_Module[i].id ) return( EW_Module[i].name );

/* Didn't find module_id in table
 ********************************/
   return( (char *)NULL );
}


         /*******************************************************
          *                      GetType                        *
          *                                                     *
          * Convert message-type name to number using table     *
          * defined in TableFile                                *
          * Return  0 on success;                               *
          *        -1 if specified message-type name is unknown *
          *******************************************************/
int GetType( char *msgName, unsigned char *msgType )
{
   int i;

   if( LoadTables ) {
       GetUtil_LoadTable();
       LoadTables = 0;
   }

/* Find message-type name in earthworm.d table
 *********************************************/
   for ( i = 0; i < Max_MessageType; i++ )
      if ( strcmp( EW_Message[i].name, msgName ) == 0 )
         break;

/* Didn't find message-type name in table
 ****************************************/
   if ( i == Max_MessageType )
   {
      getutil_logit("GetType: Invalid message-type name <%s>\n", msgName );
     *msgType = 0;
      return( -1 );
   }

/* Found it!
 ***********/
  *msgType = EW_Message[i].type;
   return( 0 );
}

         /*******************************************************
          *                   GetTypeName                       *
          *                                                     *
          * Given the numeric Message Type, return a pointer    *
          * to its character string name.  Returns NULL pointer *
          * if message type is not defined in earthworm.d       *
          *******************************************************/
char *GetTypeName( unsigned char msgtype )
{
   int i;

   if( LoadTables ) {
       GetUtil_LoadTable();
       LoadTables = 0;
   }

/* Find message-type in earthworm.d table
 ****************************************/
   for ( i = 0; i < Max_MessageType; i++ )
      if ( msgtype == EW_Message[i].type ) return( EW_Message[i].name );

/* Didn't find message-type in table
 ***********************************/
   return( (char *)NULL );
}


  /********************************************************************
   * GetUtil_LoadTable  loads the ring, module, and message tables    *
   *            from ascii files (TableFile) using kom.c functions.   *
   *            Exits if any errors are encountered.                  *
   ********************************************************************/
void GetUtil_LoadTable( void )
{
   GetUtil_LoadTableCore( 1 );
}

/* Wrapper around exit(x); if we're not aborting, we need to close the files
    & return the specified retrun code */
#define gult_exit( x ) {if ( abort ) exit( x ); \
    else {reject =1; while( nfiles > nopen )nfiles = k_close();  return x;}}

  /********************************************************************
   * GetUtil_LoadTableCore  loads the ring, module, and message tables*
   *            from ascii files (TableFile) using kom.c functions.   *
   *            If abort, exits if any errors are encountered.        *
   *               o/w returns 0 if successful, -1 if not             *
   ********************************************************************/
int GetUtil_LoadTableCore( int abort )
{
   int      ncommand;     /* # of required commands you expect to process   */
   char     init[10];     /* init flags, one byte for each required command */
   int      nmiss;        /* number of required commands that were missed   */
   char    *paramdir;    /* points to environment variable, EW_PARAMS      */
   char    *com;
   char    *str;
   int      nfiles = 0, nopen = 0;
   int      success;
   long     tmpkey;
   int      tmp;
   int      conflict;
   int      i,it;
   size_t   len;
   
   /* Counters for maximum number of (whatever) if load were successful */
   int      newMax_Ring = Max_Ring;
   int      newMax_ModuleId = Max_ModuleId;
   int      newMax_MessageType = Max_MessageType;
   int      newMax_Installation = Max_Installation;
   int      reject = 0; /* = "an error was encountered" */
   
   nfiles = 0;
   

/* Set to zero one init flag for each required command
 *****************************************************/
   ncommand = 4;
   for( i=0; i<ncommand; i++ )  init[i] = 0;

/* Get the environment variable, EW_PARAMS
 *****************************************/
   paramdir = getenv( "EW_PARAMS" ); 

   if ( paramdir == (char *)NULL )   
   {
      getutil_logit( "GetUtil_LoadTable: Environment variable EW_PARAMS not defined! EXITING!" );
      gult_exit(-1);

   }
   if ( *paramdir == '\0' )
   {
      getutil_logit( "GetUtil_LoadTable: Environment variable EW_PARAMS defined, but has no value; EXITING!\n" );
      gult_exit( -1 );
   }

/* Loop thru all interesting Table files
 ***************************************/
   for( it=0; it<nTableFile; it++ )
   {

   /* Build full path to table file
    *******************************/
      if( strlen(paramdir)+strlen(TableFile[it])+1 > (size_t)MAXLEN )
      {
         getutil_logit("GetUtil_LoadTable: length of EW_PARAMS+TableFile[%d] exceeds FullTablePath, MAXLEN=%d; EXITING!\n", it, MAXLEN );
         gult_exit( -1 );
      }
      strcpy( FullTablePath, paramdir  );
      len = strlen( FullTablePath );
   #if defined(_SOLARIS) || defined(_LINUX) || defined(_MACOSX)
      if( FullTablePath[len-1] != '/' )   strcat( FullTablePath, "/" );
   #else  /* OS/2 or NT */
      if( FullTablePath[len-1] != '\\' )  strcat( FullTablePath, "\\" );
   #endif
      strcat( FullTablePath, TableFile[it] );
      /*printf( "path to modid/msgtype table: <%s>\n", FullTablePath );*//*DEBUG*/

   /* Open the main table file
    **************************/
      nfiles = k_open( FullTablePath );
      if( nfiles == 0 ) {
           getutil_logit( "GetUtil_LoadTable: Error opening file <%s>; EXITING!\n", FullTablePath );
           gult_exit( -1 );
      }
      nopen = nfiles-1;  /* keep track of # open files before TableFile */
   
   /* Process all command files
    ***************************/
      while( nfiles > nopen ) /* While there are getutil-files open */
      {
           while(k_rd_raw(KOM_NO_EXPANSION))        /* Read next line from active file  */
           {
               com = k_str();         /* Get the first token from line */

           /* Ignore blank lines & comments
            *******************************/
               if( !com )           continue;
               if( com[0] == '#' )  continue;
   
           /* Open a nested configuration file
            **********************************/
               if( com[0] == '@' ) {
                  success = nfiles+1;
      		  strcpy( FullTablePath, paramdir  );
      		  len = strlen( FullTablePath );
   		  #if defined(_SOLARIS) || defined(_LINUX) || defined(_MACOSX)
      			if( FullTablePath[len-1] != '/' )   strcat( FullTablePath, "/" );
   		  #else  /* OS/2 or NT */
      			if( FullTablePath[len-1] != '\\' )  strcat( FullTablePath, "\\" );
   		  #endif
      		  strcat( FullTablePath, &com[1] );
                  nfiles  = k_open(FullTablePath);
                  if ( nfiles != success ) {
                     getutil_logit( "GetUtil_LoadTable: Error opening file <%s>; EXITING!\n", FullTablePath );
                     gult_exit( -1 );
                  }
                  continue;
               }

            /* Process anything else as a command:
             *************************************/
   
            /* Load shared memory ring name/key table
             ****************************************/
  /*0*/        if( k_its("Ring") ) 
               {
                /* see if there's more room in the table */
                   if ( newMax_Ring+1 >= MAX_RING ) {
                       getutil_logit( "GetUtil_LoadTable: Too many <Ring> lines in <%s>; max=%d; EXITING!\n", FullTablePath, (int) MAX_RING );
                       gult_exit( -1 );
                   }
                   str    = k_str();    /* get ring name from line */
                   tmpkey = k_long();   /* get ring key from line  */

                /* check the length of the ringname */
                   if ( strlen(str) > RINGLEN )
                   {
                       getutil_logit( "GetUtil_LoadTable: Ring name <%s> too long in <%s>; max=%d chars; exiting!\n", str, FullTablePath, RINGLEN );
                       gult_exit( -1 );
                   }

                /* look thru current table for duplicate key or name */
                   conflict = 0;
                   for( i=0; i<newMax_Ring; i++ ) {
                        if( tmpkey == EW_Ring[i].key ) {
	      		   if( strcmp( EW_Ring[i].name, str ) != 0 ) conflict=1;
                           break;
                        }
                        if( strcmp( EW_Ring[i].name, str ) == 0 ) {
                           if( tmpkey != EW_Ring[i].key ) conflict=1;
                           break;
                        }
                   }
   
                /* complain if there was a conflict with a previous setting */
                   if( conflict ) {
	                getutil_logit( "GetUtil_LoadTable: conflict with current set of Ring IDs in\n <%s>,\n New setting ignored.\n(Reconfigure does not remove existing IDs, it only adds new IDs. \nIDs must be unique.)\n", FullTablePath );
	                getutil_logit( "                   original: <Ring %s %ld>\n", EW_Ring[i].name, EW_Ring[i].key );
	                getutil_logit( "                        new: <Ring %s %ld>\n", str, tmpkey);
                    reject = 1;
                   }

                /* add new entry to table */
                   if( i==newMax_Ring ) {
                      strcpy( EW_Ring[newMax_Ring].name, str );
                      EW_Ring[newMax_Ring].key = tmpkey;
                      newMax_Ring++;
                   }
                   init[0] = 1;
               }

            /* Enter module name/id table
             ****************************/
  /*1*/        else if( k_its("Module") ) 
               {
                /* see if there's more room in the table */
                   if ( newMax_ModuleId+1 >= MAXMODID ) {
                       getutil_logit( "GetUtil_LoadTable: Too many <Module> lines in <%s>; max=%d; exiting!\n", FullTablePath, (int) MAXMODID );
                       gult_exit( -1 );
                   }
                   str = k_str();    /* get module name from line */
                   tmp = k_int();    /* get module id from line   */

                /* check validity of module id */
                   if( tmp<0 || tmp>255 ) {
                        getutil_logit( "GetUtil_LoadTable: Invalid module id <%d> in <%s> (0-255 are valid); EXITING!", tmp, FullTablePath );
		        gult_exit( -1 );
	           }     

                /* check the length of the module name */
                   if ( strlen(str) > MODLEN )
                   {
                       getutil_logit( "GetUtil_LoadTable: Module name <%s> too long in <%s>; max=%d chars; exiting!\n", str, FullTablePath, MODLEN );
                       gult_exit( -1 );
                   }

                /* look thru current table for duplicate key or name */
                   conflict = 0;
                   for( i=0; i<newMax_ModuleId; i++ ) {
                        if( tmp == (int)EW_Module[i].id ) {
		   	   if( strcmp( EW_Module[i].name, str ) != 0 ) conflict=1;
                           break;
                        }
                        if( strcmp( EW_Module[i].name, str ) == 0 ) {
                           if( tmp != (int)EW_Module[i].id ) conflict=1;
                           break;
                        }
                   }
   
                /* complain if there was a conflict with a previous setting */
                   if( conflict ) {
	                getutil_logit( "GetUtil_LoadTable: conflict with current set of Module IDs in\n <%s>,\n New setting ignored.\n(Reconfigure does not remove existing IDs, it only adds new IDs. \nIDs must be unique.)\n", FullTablePath );
	                getutil_logit( "                   original: <Module %s %d>\n", EW_Module[i].name, (int) EW_Module[i].id );
	                getutil_logit( "                        new: <Module %s %d>\n", str, tmp );
                    reject = 1;
                   }

                /* add new entry to table */
                   if( i==newMax_ModuleId ) {
                       strcpy( EW_Module[newMax_ModuleId].name, str );
                       EW_Module[newMax_ModuleId].id = (unsigned char) tmp;
                       newMax_ModuleId++;
                   }
                   init[1] = 1;
               }

            /* Enter message name/type table
             *******************************/
  /*2*/        else if( k_its("Message") ) 
               {
                /* see if there's more room in the table */
                   if ( newMax_MessageType+1 >= MAXMSGTYPE ) {
                       getutil_logit( "GetUtil_LoadTable: Too many <Message> lines in <%s> ; max=%d; exiting!\n", FullTablePath, (int) MAXMSGTYPE );
                       gult_exit( -1 );
                   }
                   str = k_str();    /* get message name from line */
                   tmp = k_int();    /* get message type from line */
   
                /* check validity of module id */
                   if( tmp<0 || tmp>255 ) {
                        getutil_logit( "GetUtil_LoadTable: Invalid message type <%d> in <%s> (0-255 are valid); exiting!\n", tmp, FullTablePath );
		        gult_exit( -1 );
	           }     
                   
                /* check the length of the message */
                   if ( strlen(str) > MSGLEN )
                   {
                       getutil_logit( "GetUtil_LoadTable: Message name <%s> too long in <%s>; max=%d chars; exiting!\n", str, FullTablePath, MSGLEN );
                       gult_exit( -1 );
                   }

                /* look thru current table for duplicate type or name */
                   conflict = 0;
                   for( i=0; i<newMax_MessageType; i++ ) {
                        if( tmp == (int)EW_Message[i].type ) {
			   if( strcmp( EW_Message[i].name, str ) != 0 ) conflict=1;
                           break;
                        }
                        if( strcmp( EW_Message[i].name, str ) == 0 ) {
                           if( tmp != (int)EW_Message[i].type ) conflict=1;
                           break;
                        }
                   }
   
                /* complain if there was a conflict with a previous setting */
                   if( conflict ) {
	                getutil_logit( "GetUtil_LoadTable: conflict with current set of Message Type IDs in\n <%s>,\n New setting ignored.\n(Reconfigure does not remove existing IDs, it only adds new IDs. \nIDs must be unique.)\n", FullTablePath );
	                getutil_logit( "                   original: <Message %s %d>\n", EW_Message[i].name, (int) EW_Message[i].type );
	                getutil_logit( "                        new: <Message %s %d>\n", str, tmp );
                    reject = 1;
                   }

                /* add new entry to table */
                   if( i==newMax_MessageType ) {
                       strcpy( EW_Message[newMax_MessageType].name, str );
                       EW_Message[newMax_MessageType].type = (unsigned char) tmp;
                       newMax_MessageType++;
                   }
                   init[2] = 1;
               }

            /* Enter installation name/type table
             ************************************/
  /*3*/        else if( k_its("Installation") ) 
               {
                /* see if there's more room in the table */
                   if ( newMax_Installation+1 >= MAXINSTID ) {
                       getutil_logit( "GetUtil_LoadTable: Too many <Installation> lines in <%s>; max=%d; exiting!\n", FullTablePath, (int) MAXINSTID );
                       gult_exit( -1 );
                   }
                   str = k_str();    /* get installation name from line */
                   tmp = k_int();    /* get instid from line */
   
                /* check validity of instid */
                   if( tmp<0 || tmp>255 ) {
                        getutil_logit( "GetUtil_LoadTable: Invalid installation id <%d> in <%s> (0-255 are valid); exiting!\n", tmp, FullTablePath );
		        gult_exit( -1 );
	           }     
                   
                /* check the length of the installation name */
                   if ( strlen(str) > INSTLEN )
                   {
                       getutil_logit( "GetUtil_LoadTable: Installation name <%s> too long in <%s>; max=%d chars; exiting!\n", str, FullTablePath, INSTLEN );
                       gult_exit( -1 );
                   }

                /* look thru current table for duplicate instid or name */
                   conflict = 0;
                   for( i=0; i<newMax_Installation; i++ ) {
                        if( tmp == (int)EW_Installation[i].id ) {
			   if( strcmp( EW_Installation[i].name, str ) != 0 ) conflict=1;
                           break;
                        }
                        if( strcmp( EW_Installation[i].name, str ) == 0 ) {
                           if( tmp != (int)EW_Installation[i].id ) conflict=1;
                           break;
                        }
                   }
   
                /* complain if there was a conflict with a previous setting */
                   if( conflict ) {
	                getutil_logit( "GetUtil_LoadTable: conflict with current set of Installation IDs in\n <%s>,\n New setting ignored.\n(Reconfigure does not remove existing IDs, it only adds new IDs. \nIDs must be unique.)\n",  FullTablePath );
	                getutil_logit( "                   original: <Installation %s %d>\n", EW_Installation[i].name, (int) EW_Installation[i].id );
	                getutil_logit( "                        new: <Installation %s %d>\n", str, tmp );
                    reject = 1;
                   }

                /* add new entry to table */
                   if( i==newMax_Installation ) {
                       strcpy( EW_Installation[newMax_Installation].name, str );
                       EW_Installation[newMax_Installation].id = (unsigned char) tmp;
                       newMax_Installation++;
                   }
                   init[3] = 1;
               }

            /* Otherwise, it's unknown
             *************************/
               else  
               {
                   getutil_logit( "GetUtil_LoadTable: <%s> Unknown command in <%s>.\n", com, FullTablePath );
                   continue;
               }

           /* See if there were any errors processing the command
            *****************************************************/
               if( k_err() ) 
               {
                  getutil_logit( "GetUtil_LoadTable: Bad <%s> line in <%s>; exiting!\n", com, FullTablePath );
                  gult_exit( -1 );
               }
           }
           nfiles = k_close();
      }
   } /* end-for over all table files */

/* After all files are closed, check init flags for missed commands
 ******************************************************************/
   nmiss = 0;
   for ( i=0; i<ncommand; i++ )  if( !init[i] ) nmiss++;
   if ( nmiss ) {
       getutil_logit( "GetUtil_LoadTable: ERROR, no "    );
       if ( !init[0] )  getutil_logit( "<Ring> "         );
       if ( !init[1] )  getutil_logit( "<Module> "       );
       if ( !init[2] )  getutil_logit( "<Message> "      );
       if ( !init[3] )  getutil_logit( "<Installation> " );
       getutil_logit( "line(s) in file(s) " );
       for( it=0; it<nTableFile; it++ ) getutil_logit( "<%s> ", TableFile[it] );
       getutil_logit( "exiting!\n" );
       gult_exit( -1 );
   }

       
/* Now that we know all is well, adjust Max_* to reflect additions
 *****************************************************************/
   Max_Ring = newMax_Ring;
   Max_ModuleId = newMax_ModuleId;
   Max_MessageType = newMax_MessageType;
   Max_Installation = newMax_Installation;
 
   /* Moved this below the preceeding four lines as if we're just ignoring 
    * the problem, we do still want any valid new rings/modules/messages/insts
    * to take effect - Stefan 20121030 */
   if ( reject ) 
       return -1;

   return 0;
}
