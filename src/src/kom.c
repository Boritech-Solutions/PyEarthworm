
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: kom.c 6381 2015-06-13 14:11:41Z paulf $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.6  2008/03/05 13:02:23  quintiliani
 *     Minor change: substituted comment // to /star star/
 *
 *     Revision 1.5  2008/03/05 08:20:52  quintiliani
 *     Minor change: substituted comment // to /star star/
 *
 *     Revision 1.4  2007/07/25 23:58:08  hal
 *     * config reading (from files and strings) is now line-termination-agnostic
 *     * k_rd() and k_put() have been merged, as most of their code was identical
 *
 *     Revision 1.3  2004/03/22 21:20:36  kohler
 *     Changed "static struct K_buf" to "struct k_buf"
 *
 *     Revision 1.2  2001/10/05 20:42:15  dietz
 *     Increased MAXCRD to 1024 (was 512)
 *
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */

/*
 * kom.c : Simple positional command parser.
 *
 *$ 91May07 CEJ Version 1.0
 *$ 93Oct03 CEJ Added k_put routine.
 *$ 95Oct18 LDD Created kom.h to house function prototypes.
 *              Explicitly declared return types for all functions.
 */
/*********************C O P Y R I G H T   N O T I C E ***********************/
/* Copyright 1991 by Carl Johnson.  All rights are reserved. Permission     */
/* is hereby granted for the use of this product for nonprofit, commercial, */
/* or noncommercial publications that contain appropriate acknowledgement   */
/* of the author. Modification of this code is permitted as long as this    */
/* notice is included in each resulting source module.                      */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <kom.h>

#define BOOL_EXPANSION_FROM_ENV(x)  ( (x==KOM_EXPANSION_FROM_ENV)? 1 : 0 )
#define BOOL_EXPANSION_FROM_FILE(x) ( (x==KOM_EXPANSION_FROM_FILE)? 1 : 0 )

int validate_envvar_name(const char *envvar_name); /* do no use logit */
int LoadTableEnvVariable(); /* use logit */
char *GetEnvVarValueFromFile( const char *envvar_name ); /* use logit by LoadTableEnvVariable() */
char *k_getenv(const char *envvar_name, int flag_expand_from_file); /* use logit */
int k_envvar_expansion(char *dst, const char *src, size_t max_dst, int flag_expand_from_file); /* use logit by k_getenv */

/* default function for logging messages */
void kom_log_func_default(char *message) {
    fprintf(stderr,"%s",message);
}

/* function pointer to the current function for logging messages */
static void (*kom_log_func)(char *) = kom_log_func_default;

/* Change function for logging messages, default is fprintf to stderr */
void set_kom_log(void (*par_kom_log_func)(char *) )
{
    kom_log_func = par_kom_log_func;
}

#define KOM_MAX_LOG_MESSAGE_LENGTH 4096
/* function to call within kom for logging messages */
void kom_log(char *format, ... )
{
    va_list listptr;
    int retvalue = 0;
    char message[KOM_MAX_LOG_MESSAGE_LENGTH];

    if(kom_log_func != NULL) {
      va_start(listptr, format);
      retvalue = vsnprintf(message, (size_t) KOM_MAX_LOG_MESSAGE_LENGTH, format, listptr);
      kom_log_func(message);
      va_end(listptr);
    }
}

#define MAXCRD 2048
struct k_buf {
        FILE *fid;
        int ncrd;               /* bytes in command             */
        int istr;               /* index of current string      */
        int icrd;               /* scan pointer                 */
        int ierr;               /* error if non-zero            */
        char crd[MAXCRD];       /* command buffer               */
        char csav[MAXCRD];      /* last command read            */
} ;

#define MAXBUF 4
struct k_buf com = {0L, 0, 0, 0, 0, "Virgin", "Virgin"};
struct k_buf Kbuf[MAXBUF];
int Nbuf = 0;
char iniFormat = 0;

/*
 **** k_open_base : common code for k_open & k_open_format
 */
static int k_open_base( char *name )
{
        if(Nbuf < MAXBUF && com.fid) {
                Kbuf[Nbuf++] = com;
                com.fid = 0;
        }
        if(com.fid)
                return(0);
/*      if(!strcmp(name, "term"))
                com.fid = stdin;
        else                                                    */
                com.fid = fopen(name, "r");
        if(com.fid)
                return(Nbuf+1);
        return(0);

}

/*
 **** k_open : open new file for k-system input.  For now only one file
 *              can be open at a time, but it should be straight foward to add
 *              recursion capabitity using a stack in the com area.
 */
int k_open( char *name )
{
	iniFormat = 0;
	return k_open_base( name );
}

/*
 **** k_open_format : k_open, but specify file format
 *                    ewFormat=0 means file is in .ini format
 *                               o/w use normal EW format
 */
int k_open_format( char *name, char ewFormat )
{
	iniFormat = ewFormat ? 0 : '=';
	return k_open_base( name );
}

/*
 **** k_treat_as_ini : From now on, treat the currently open file as ini format.
                       If no file is open, do nothing
 */
void k_treat_as_ini( void )
{
	if(com.fid)
		iniFormat = '=';
}

/*
 **** k_treat_as_ew : From now on, treat the currently open file as ew format.
                      If no file is open, do nothing
 */
void k_treat_as_ew( void )
{
	if(com.fid)
		iniFormat = 0;
}

/*
 **** k_close : close current file
 */
int k_close( void )
{
        if(com.fid){
                if(com.fid != stdin)
                        fclose(com.fid);
                com.fid = 0L;
        }
        if(Nbuf > 0) {
                com = Kbuf[--Nbuf];
                return Nbuf+1;
        }
        return(0);

}

/*
 **** k_get : Return pointer to current card.
 */
char *k_get( void )
{
        return com.crd;
}

/*
 **** k_dump : Print last card read.
 */
void k_dump( void )
{
        printf("%s\n", com.crd);
}

/*
 **** k_err : return last error code and clear
 */
int k_err( void )
{
        int jerr;

        jerr = com.ierr;
        com.ierr = 0;
        return(jerr);
}

/*
 * k_int : Parce next token as integer.
 */
int k_int( void )
{
        int ival;
        char *s;

        s = k_str();
        if(!s) {
                com.ierr = -1;
                return(0);
        }
        ival = atoi(s);
        if(ival == 0 && *s != '0') {
                com.ierr = -2;
                return(0);
        }
        return(ival);
}

/*
 * k_its : Compare string from last token to given command.
 */
int k_its(char *c)
{
        char *s;

        s = &com.crd[com.istr];
        while(*c == *s) {
                if(*s == '\0')
                        return(1);
                c++;
                s++;
        }
        return(0);
}

/*
 * k_long : Return next token as a long integer.
 */
long k_long( void )
{
        long lval;
        char *s;

        s = k_str();
        if(!s) {
                com.ierr = -1;
                return(0L);
        }
        lval = atol(s);
        if(lval == 0 && *s != '0') {
                com.ierr = -2;
               return(0L);
        }
        return(lval);
}

/*
 **** k_put : insert command line into buffer
 */
int k_put(char *crd)
 {
        int i, n;

        strcpy(com.crd, crd);
        com.ncrd = strlen(crd);
        /* Lets account for both unix and windows line termination */	
        /* if(com.ncrd && com.crd[com.ncrd-1] == '\n') */
        /*         com.crd[--com.ncrd] = 0;            */
        while (com.ncrd && ( com.crd[com.ncrd-1] == '\r' || com.crd[com.ncrd-1] == '\n')) {
            com.crd[--com.ncrd] = 0;
        }
        if(!com.ncrd) {
                com.ncrd = 1;
                com.crd[0] = ' ';
                com.crd[1] = 0;
        }
        com.istr = 0;
        com.icrd = 0;
        com.ierr = 0;
        n = 1;
        for(i=0; i<com.ncrd; i++) {
                if(com.crd[i] == '\t')
                        com.crd[i] = ' ';
                if(com.crd[i] != ' ')
                        n = i + 1;
        }
        com.ncrd = n;
        com.crd[n] = 0;
        strcpy(com.csav, com.crd);
        return(com.ncrd);
}

/*
 **** k_rd_raw : read command line into buffer and try to expand environment variable
 *                     if flag_expansion_level is true, expansion from environment and file
 *                     otherwise only from environment
 */
int k_rd_raw( int flag_expansion_level ) {
	int ret;
	extern struct k_buf com;
	char newcrd[MAXCRD];
	char newcrd_envvar_exp[MAXCRD];

	if(com.fid) {
	  if(!fgets(newcrd, MAXCRD-1, com.fid)) {
	    if(feof(com.fid)) {
	      return 0;
	    }
	  }
	}

	/* POSSIBLE BUG EVEN THOUGH I AM NOT COMPLETELY SURE:
	 * Since the current limit passed to fgets() is equal to MAXCRD then
	 * line longer than MAXCRD could be truncated. As a consequence, some
	 * environment variable names could be truncated and not expanded.
	 * POSSIBLE SOLUTION:
	 * increase the value of MAXCRD for read newcrd by fgets(). */

	/* Environment variable expansion */
	if(flag_expansion_level == KOM_NO_EXPANSION) {
	    /* backward compatibility, old behavior */
	    ret =  k_put(newcrd);
	} else {
	    if( k_envvar_expansion(newcrd_envvar_exp, newcrd, (size_t) MAXCRD,
			BOOL_EXPANSION_FROM_FILE(flag_expansion_level)) == 0 ) {
		ret =  k_put(newcrd_envvar_exp);
	    } else {
		kom_log("warning: skipping variable expansion for the following line (env var match not found):\n%s", newcrd);
		ret =  k_put(newcrd);
	    }
	}

	return ret;
}


/*
 **** k_rd : read command line into buffer
 */
int k_rd( void ) {
    /* Read next line from active file with the variable expansion from environment and file */
	if ( iniFormat )
		iniFormat = '=';
    return k_rd_raw(KOM_EXPANSION_FROM_FILE);
}


/*
 **** k_com : returns last command line read
 */
char *k_com( void )
{
        return com.csav;
}

/*
 **** k_str() : Return next token as a pointer to string.
 */
char *k_str( void )
{
        int state;
        int i;

        state = 1;
        for(i=com.icrd; i<com.ncrd; i++) {
                switch(state) {
                case 1: /* Looking for first non-blank          */
                        if(com.crd[i] == ' ' || com.crd[i] == '\t')
                                break;
                        if(com.crd[i] == '"') {
                                state = 3;
                                com.istr = i + 1;
                                break;
                        }
                        if(com.crd[i] == '[') {
                                state = 4;
                                com.istr = i;
                                break;
                        }
                        state = 2;
                        com.istr = i;
                        break;
                case 2: /* Looking for end of normal string */
                		if(iniFormat) {
                			if (iniFormat == com.crd[i]) {
                				int j = i-1;
                				while ( j>0 && (com.crd[j] == ' ' || com.crd[j] =='\t') )
                					j--;
                				com.crd[j+1] = 0;
                				com.icrd = i + 1;
                				iniFormat = ',';
                				return(&com.crd[com.istr]);
                			}
                			continue;
                		}
                        if(com.crd[i] == ' ' || com.crd[i] == '\t') {
                                com.crd[i] = 0;
                                com.icrd = i + 1;
                                return(&com.crd[com.istr]);
                        }
                        break;
                case 3: /* Quoted string */
                        if(com.crd[i] == '"') {
                                com.crd[i] = 0;
                                com.icrd = i + 1;
                                return(&com.crd[com.istr]);
                        }
                        break;
                case 4: /* Bracketed string */
                        if(com.crd[i] == ']') {
                                com.crd[i] = 0;
                                com.icrd = i + 1;
                                return(&com.crd[com.istr]);
                        }
                        break;
                }
        }
        if(state == 2) {
                com.crd[com.ncrd] = 0;
                com.icrd = com.ncrd;
                return(&com.crd[com.istr]);
        }
        com.ierr = -17;
        return( (char *) 0 );
}

/*
 **** k_val() Return next token as a double real
 */
double k_val( void )
{
        double val;
        char *s;

        s = k_str();
        if(!s) {
                com.ierr = -1;
                return(0.0);
        }
        val = atof(s);
        return(val);
}



/*************************************************************************
 * validate_envvar_name  validate if envvar_name is a valid identifier   *
 *            Return true or false.                                      *
 *************************************************************************/
int validate_envvar_name(const char *envvar_name) {
    int ret;
    int j = 0;
    int l = strlen(envvar_name);

    /* assume it is not ok */
    ret = 0;

    if(l > 0) {

	/* starts with a character [A-Za-z] */
	if(
		(envvar_name[0] >= 'A'  &&  envvar_name[0] <= 'Z')
		||
		(envvar_name[0] >= 'a'  &&  envvar_name[0] <= 'z')
	  ) {

	    /* assume it is ok */
	    ret = 1;
	    while(ret  &&  j < l) {
		if( !(
			    (envvar_name[j] >= 'A'  &&  envvar_name[j] <= 'Z')
			    ||
			    (envvar_name[j] >= 'a'  &&  envvar_name[j] <= 'z')
			    ||
			    (envvar_name[j] >= '0'  &&  envvar_name[j] <= '9')
			    ||
			    (envvar_name[j] == '_')
		     )
		  ) {
		    ret = 0;
		}
		j++;
	    }

	}

    }

    return ret;
}


/* Table of Environment variable names and their values
 ***********************************************/
#define MAXENVVAR 256
#define ENVVARLEN 255
static struct {
  char          name[ENVVARLEN+1];
  char          value[ENVVARLEN+1];
} EW_EnvVariable[MAXENVVAR];
static int Max_EnvVariable = 0;  /* # Environment variable currently in table */
static int flag_LoadTableEnvVariable = 1;

int LoadTableEnvVariable() {
    int      newMax_EnvVariable = Max_EnvVariable;
    char     *com;
    char     *str_value;
    char     *str;
    int      skip = 0; /* = "an warning was encountered" */
    int      nfiles = 0;
    int      nopen = 0;
    char     *paramdir;    /* points to environment variable, EW_PARAMS      */
    int      len;
    // int      success;
    int      i;
    FILE     *f = NULL;

    const char configfile_commonvars[] = "earthworm_commonvars.d";
#define MAXFILELENCOMVARS 512
    static char  fullpath_configfile_commonvars[MAXFILELENCOMVARS+1];

    /* Build fullpath_configfile_commonvars */
    paramdir = getenv( "EW_PARAMS" ); 
    strcpy( fullpath_configfile_commonvars, paramdir  );
    len = strlen( fullpath_configfile_commonvars );
#if defined(_SOLARIS) || defined(_LINUX) || defined(_MACOSX)
    if( fullpath_configfile_commonvars[len-1] != '/' )   strcat( fullpath_configfile_commonvars, "/" );
#else  /* OS/2 or NT */
    if( fullpath_configfile_commonvars[len-1] != '\\' )  strcat( fullpath_configfile_commonvars, "\\" );
#endif
    strcat( fullpath_configfile_commonvars, configfile_commonvars );

    f = fopen (fullpath_configfile_commonvars, "rt");
    if(f) {
	fclose(f);
	/* Open the main configuration file */
	nfiles = k_open_base (fullpath_configfile_commonvars);
	if (nfiles == 0) {
	    kom_log("LoadTableEnvVariable(): Error opening command file <%s>; exiting!\n", 
		    configfile_commonvars);
	    return -1;
	}
	nopen = nfiles-1;  /* keep track of # open files before ... */
    }  else {
	kom_log("LoadTableEnvVariable(): warning %s file not found.\n", fullpath_configfile_commonvars);

    }

    /* Process all command files */
    while (nfiles > nopen) {   /* While there are command files open */

	kom_log("LoadTableEnvVariable(): nfiles %d\n", nfiles);

	/* Read file without expanding environment variable from file */
	while (k_rd_raw(KOM_NO_EXPANSION)) {       /* Read next line from active file without expansion from file,
				       this prevents infinite recursivity on itself */
	    com = k_str ();         /* Get the first token from line */

	    /* Ignore blank lines & comments */
	    if (!com)
		continue;
	    if (com[0] == '#')
		continue;

	    /* Open a nested configuration file */
	    /*
	    if (com[0] == '@') {
		success = nfiles + 1;
		nfiles  = k_open_base (&com[1]);
		if (nfiles != success) {
		    kom_log("LoadTableEnvVariable(): Error opening command file <%s>; exiting!\n",
			    &com[1]);
		    return -1;
		}
		continue;
	    }
	    */

	    /* Enter environment variable name/value table
	     *******************************/
	    if( k_its("SetEnvVariable") ) 
	    {
		skip = 0;
		/* see if there's more room in the table */
		if ( newMax_EnvVariable+1 >= MAXENVVAR ) {
		    kom_log("LoadTableEnvVariable(): Too many <SetEnvVariable> lines in <%s>",
			    configfile_commonvars );
		    kom_log("; max=%d; exiting!\n", (int) MAXENVVAR );
		    return -1;
		}
		str = k_str();          /* get variable name from line */
		str_value = k_str();    /* get variable value from line */

		/* check validity of variable name */
		if( !validate_envvar_name(str) ) {
		    kom_log("LoadTableEnvVariable(): Invalid environment variable name <%s> in <%s>",
			    str, configfile_commonvars );
		    kom_log(" (names without spaces are valid); skipping!\n" );
		    skip = 1;
		}     


		/* check NULL of the environment variable name */
		if ( !str && !skip)
		{
		    kom_log("LoadTableEnvVariable(): command EnvVariable NULL for name in <%s>;"
			    " max=%d chars; exiting!\n", configfile_commonvars, ENVVARLEN );
		    return -1;
		}

		/* check NULL of the environment variable value */
		if ( !str_value  && !skip)
		{
		    kom_log("LoadTableEnvVariable(): command EnvVariable NULL for value of %s in <%s>;"
			    " max=%d chars; exiting!\n", str, configfile_commonvars, ENVVARLEN );
		    return -1;
		}

		/* check the length of the environment variable name */
		if ( strlen(str) > ENVVARLEN  && !skip)
		{
		    kom_log("LoadTableEnvVariable(): environment variable name <%s> too long in <%s>;"
			    " max=%d chars; exiting!\n", str, configfile_commonvars, ENVVARLEN );
		    return -1;
		}

		/* check the length of the environment variable value */
		if ( strlen(str_value) > ENVVARLEN  && !skip)
		{
		    kom_log("LoadTableEnvVariable(): environment variable value <%s> too long in <%s>;"
			    " max=%d chars; exiting!\n", str_value, configfile_commonvars, ENVVARLEN );
		    return -1;
		}


		/* look thru current table for duplicate type or name */
		if(!skip) {
		    i = 0;
		    while(i<newMax_EnvVariable  &&  !skip) {
			if( strcmp( EW_EnvVariable[i].name, str ) == 0 ) {
			    skip=1;
			} else {
			    i++;
			}
		    }

		    /* complain if there was a duplication with a previous setting */
		    if( skip ) {
			kom_log("LoadTableEnvVariable(): duplication variable in <%s>, new setting ignored\n", 
				configfile_commonvars );
			kom_log("                   original: <EnvVariable %s='%s'>\n", 
				EW_EnvVariable[i].name, EW_EnvVariable[i].value );
			kom_log("                        new: <EnvVariable %s='%s'>\n", 
				str, str_value );
			skip = 1;
		    }

		}

		/* add new entry to table */
		if( i==newMax_EnvVariable  &&  !skip) {
		    strncpy( EW_EnvVariable[newMax_EnvVariable].name, str, (size_t) ENVVARLEN );
		    strncpy( EW_EnvVariable[newMax_EnvVariable].value, str_value, (size_t) ENVVARLEN );
		    newMax_EnvVariable++;
		}
	    }

	    /* Unknown command */
	    else {
		kom_log("LoadTableEnvVariable(): <%s> Unknown command in <%s>.\n",
			com, fullpath_configfile_commonvars);
		continue;
	    }

	    /* See if there were any errors processing the command */
	    if (k_err ()) {
		kom_log("LoadTableEnvVariable(): Bad command in <%s>; exiting!\n\t%s\n",
			fullpath_configfile_commonvars, k_com());
		return -1;
	    }

	} /** while k_rd_raw() **/

	nfiles = k_close ();

    } /** while nfiles **/

    Max_EnvVariable = newMax_EnvVariable;

    /* TODO Expand further value ?*/

#ifdef DEBUG_ENVTABLE
    kom_log("LoadTableEnvVariable(): EW_EnvVariable current table: %d items\n", Max_EnvVariable);
    for(i=0; i < Max_EnvVariable; i++) {
	kom_log("LoadTableEnvVariable(): %d %s=\"%s\"\n",
		i+1, EW_EnvVariable[i].name, EW_EnvVariable[i].value);
    }
#endif

    return 0;
}

/********************************************************
*               GetEnvVarValueFromFile                 *
*                                                      *
* Given an environment variable name, return a pointer *
* to its character string value.  Returns NULL pointer *
* if the environment variable is not defined in        *
* earthworm_commonvars.d                               *
********************************************************/
char *GetEnvVarValueFromFile( const char *envvar_name ) {
    int i;
    char *ret = NULL;

    if(envvar_name) {

	/* Load environment variable into table when it is needed */
	if( flag_LoadTableEnvVariable ) {
	    LoadTableEnvVariable();
	    flag_LoadTableEnvVariable = 0;
	}


	/* Find environment variable value in earthworm.d table
	 **************************************/
	i = 0;
	while( i <  Max_EnvVariable  &&  !ret ) {
	    if ( strcmp(EW_EnvVariable[i].name, envvar_name) == 0 ) {
		ret  =  EW_EnvVariable[i].value;
	    }
	    i++;
	}

    }

    return ret;
}


/*
 **** k_getenv :
 *    Obtains the current value of the environment Earthworm variable, envvar_name
 *               if flag_expand_from_file is true, expansion from environment and file
 *               otherwise only from environment
 */
char *k_getenv(const char *envvar_name, int flag_expand_from_file) {
    char *ret;
    /* 0 environment getenv(), 1 earthworm file GetEnvVarValueFromFile() */
    int source = 0;
    char *value_from_file = NULL;
    char *value_from_environment = NULL;

    /* TODO explain order used for getting variable values */
    value_from_environment = getenv( envvar_name );
    if(flag_expand_from_file) {
	value_from_file = GetEnvVarValueFromFile( envvar_name );
	if(value_from_file) {
	    source = 1;
	    ret = value_from_file;
	} else {
	    ret = value_from_environment;
	}
    } else {
	ret = value_from_environment;
    }

/* commented out this logging of where env came from to stderr, as it was getting cumbersome, on 2013-07-13 */

/* perhaps re-enable this with an environment debug flag somehow?
    if(ret) {
	kom_log("Environment variable %s=\"%s\" read from %s\n",
		envvar_name, ret, (source==0)? "Environment" : "File");
    } else {
	kom_log("Environment variable %s not found (from file %s).\n",
		envvar_name, (flag_expand_from_file)? "enabled" : "disabled");
    }
*/

    return ret;
}

/*
 **** k_envvar_expansion :
 *    Copies at most max_dst-1 characters from src into dst replacing
 *    environment variable name declared by the syntax ${.....} with respective value.
 *        src is the input buffer as NULL terminated-string
 *        dst is the output buffer as NULL terminated-string
 *        flag_expand_from_file is the flag passed to k_getenv()
 *    Return -1 in case of failure (do not use dst),
 *            0 otherwise (you can use dst)
 *
 */
int k_envvar_expansion(char *dst, const char *src, size_t max_dst, int flag_expand_from_file) {
    int ret = 0;
    int flag_copy = 0;
    char envvar_name[MAXCRD];
    char envvar_value[MAXCRD];
    int j, k;
    char *temp_variable_value;
    int i_src = 0;
    int i_dst = 0;
    int l_src = strlen(src);
    int count_global_var   = 0;
    int count_expanded_var = 0;
    int comment_started = 0;

    while(i_src < l_src  &&  ret == 0) {
	flag_copy = 1;

	/* Check if more space available for string dst */
	if(i_dst+1 < max_dst) {

	    /* Take precaution against possible multi-lines within the src buffer */
	    if(src[i_src] == '\n'  ||  src[i_src] == '\r') {
		comment_started = 0;
	    /* Start a comment. TODO: pound symbol could belong to a previous value */
	    } else if(src[i_src] == '#') {
		comment_started = 1;
	    }

	    /* Check for Copying or Expanding */
	    if(src[i_src] == '$'  &&  !comment_started) {
		if(i_src+1<l_src  &&  src[i_src+1] == '{') {

		    /* Look for '}' */
		    j = i_src + 2;
		    while(j < l_src  &&  src[j] != '}') {
			j++;
		    }

		    /* TODO variable name can not contain '}' */
		    if(j < l_src  &&  src[j] == '}') {
			/* Expand environment variable */
			flag_copy = 0;
			count_global_var++;

			/* Set envvar_name */
			k=i_src+2;
			while(k<j) {
			    envvar_name[k - (i_src+2)] = src[k];
			    k++;
			}
			envvar_name[k - (i_src+2)] = 0;

			/* Set envvar_value from  envvar_name */
			temp_variable_value = k_getenv( envvar_name, flag_expand_from_file );
			if(temp_variable_value) {
			    count_expanded_var++;
			    strncpy(envvar_value, temp_variable_value, (size_t) MAXCRD);
			} else {
			    /* Not expand value and set again flag_copy to 1 */
			    flag_copy = 1;
			    envvar_value[0] = 0;
			}

		    }

		    /* Expand environment variable value into dst */
		    if(!flag_copy) {
			i_src = j;
			k=0;
			while(k<strlen(envvar_value)  &&  ret == 0 ) {
			    if(i_dst+1 < max_dst) {
				dst[i_dst] = envvar_value[k];
				i_dst++;
			    } else {
				ret = -1;
			    }
			    k++;
			}
		    }

		}
	    }

	    /* Copy current character */
	    if(flag_copy) {
		dst[i_dst] = src[i_src];
		i_dst++;
	    }

	} else {
	    /* error: over bound destination string */
	    ret = -1;
	}

	i_src++;
    }

    /* Terminate destination string */
    dst[i_dst] = 0;
    i_dst++;

    /* At least one of the found variables has to have been expanded */
    if(count_global_var>0 && count_expanded_var==0) {
	ret = -1;
    }

    return ret;
}

