# file: rdpickcoda.h 

cdef extern from "inc/rdpickcoda.h":

  ctypedef struct EWPICK:
    unsigned char msgtype;
    unsigned char modid;
    unsigned char instid;
    int    seq;
    char   site[TRACE2_STA_LEN];
    char   net[TRACE2_NET_LEN];
    char   comp[TRACE2_CHAN_LEN];
    char   loc[TRACE2_LOC_LEN];
    char   fm;
    char   wt;
    double tpick;
    long   pamp[3];

  ctypedef struct EWCODA:
    unsigned char msgtype;
    unsigned char modid;
    unsigned char instid;
    int    seq;
    char   site[TRACE2_STA_LEN];
    char   net[TRACE2_NET_LEN];
    char   comp[TRACE2_CHAN_LEN];
    char   loc[TRACE2_LOC_LEN];
    long   caav[6];
    int    dur;

  int  rd_pick2k( char *msg, int msglen, EWPICK *pk );
  int  rd_coda2k( char *msg, int msglen, EWCODA *cd );
  int  rd_pick_scnl( char *msg, int msglen, EWPICK *pk );
  int  rd_coda_scnl( char *msg, int msglen, EWCODA *cd );

