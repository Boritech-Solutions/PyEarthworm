/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: rdpickcoda.h 1469 2004-05-14 17:57:01Z dietz $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2004/05/14 17:57:01  dietz
 *     moved from pkfilter's source dir
 *
 *     Revision 1.2  2004/04/29 22:01:07  dietz
 *     added capability to process TYPE_PICK_SCNL and TYPE_CODA_SCNL msgs
 *
 *     Revision 1.1  2004/04/22 18:01:56  dietz
 *     Moved pkfilter source from Contrib/Menlo to the earthworm orthodoxy
 *
 *
 *
 */

/* rdpickcoda.h
 *
 * Header file for the functions in rdpickcoda.c that convert
 * from TYPE_PICK2K, TYPE_PICK_SCNL, TYPE_CODA2K, TYPE_CODA_SCNL
 * messages to structures.
 * 
 * written by Lynn Dietz   January, 2002
 */

#ifndef RDPICKCODA_H
#define RDPICKCODA_H

#include <trace_buf.h>

/* Structure to contain data from a TYPE_PICK2K or TYPE_PICK_SCNL
   In the comments below, NTS = Null Terminated String
 ****************************************************************/
typedef struct _EWPICK {
   unsigned char msgtype;        /* one-byte number message type         */
   unsigned char modid;          /* one-byte number module id            */
   unsigned char instid;         /* one-byte number installation id      */
   int    seq;                   /* sequence number                      */
   char   site[TRACE2_STA_LEN];  /* NTS: Site code as per IRIS SEED      */
   char   net[TRACE2_NET_LEN];   /* NTS: Network code as per IRIS SEED   */
   char   comp[TRACE2_CHAN_LEN]; /* NTS: Component code as per IRIS SEED */
   char   loc[TRACE2_LOC_LEN];   /* NTS: location code as per IRIS SEED  */
                                 /*      set to "--" for TYPE_PICK2K     */
   char   fm;                    /* first-motion descriptor (U,D,' ','?')*/
   char   wt;                    /* pick weight or quality (0-4)         */
   double tpick;                 /* time of pick - seconds since 1970    */
   long   pamp[3];               /* P amplitudes in digital counts       */
} EWPICK;

/* Structure to contain data from a TYPE_CODA2K or TYPE_CODA_SCNL
   In the comments below, NTS = Null Terminated String
 ****************************************************************/
typedef struct _EWCODA {
   unsigned char msgtype;        /* one-byte number message type         */
   unsigned char modid;          /* one-byte number module id            */
   unsigned char instid;         /* one-byte number installation id      */
   int    seq;                   /* sequence number                      */
   char   site[TRACE2_STA_LEN];  /* NTS: Site code as per IRIS SEED      */
   char   net[TRACE2_NET_LEN];   /* NTS: Network code as per IRIS SEED   */
   char   comp[TRACE2_CHAN_LEN]; /* NTS: Component code as per IRIS SEED */
   char   loc[TRACE2_LOC_LEN];   /* NTS: location code as per IRIS SEED  */
                                 /*      set to "--" for TYPE_CODA2K     */
   long   caav[6];               /* coda avg abs value (counts) 2s window*/
   int    dur;                   /* coda duration (seconds since pick)   */
} EWCODA;


/* Function Prototypes
 *********************/
int  rd_pick2k( char *msg, int msglen, EWPICK *pk );
int  rd_coda2k( char *msg, int msglen, EWCODA *cd );
int  rd_pick_scnl( char *msg, int msglen, EWPICK *pk );
int  rd_coda_scnl( char *msg, int msglen, EWCODA *cd );

#endif
