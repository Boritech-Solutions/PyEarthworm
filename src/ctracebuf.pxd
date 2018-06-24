# file: trace_buf.h 

cdef extern from "inc/trace_buf.h":
  cdef const int	TRACE_STA_LEN
  cdef const int	TRACE_CHAN_LEN
  cdef const int	TRACE_NET_LEN	
  cdef const int  TRACE_LOC_LEN
  cdef const int	TRACE2_STA_LEN
  cdef const int	TRACE2_NET_LEN
  cdef const int	TRACE2_CHAN_LEN
  cdef const int	TRACE2_LOC_LEN

  cdef char TRACE2_VERSION0  
  cdef char TRACE2_VERSION1  
  cdef char TRACE2_VERSION11 
  
  ctypedef struct TRACE_HEADER:
    int     pinno;
    int     nsamp;
    double  starttime;
    double  endtime;
    double  samprate;
    char    sta[TRACE_STA_LEN];
    char    net[TRACE_NET_LEN];
    char    chan[TRACE_CHAN_LEN];
    char    datatype[3];
    char    quality[2];
    char    pad[2];


  ctypedef struct TRACE2_HEADER:
    int     pinno;
    int     nsamp;
    double  starttime;
    double  endtime;
    double  samprate;
    char    sta[TRACE2_STA_LEN];
    char    net[TRACE2_NET_LEN];
    char    chan[TRACE2_CHAN_LEN];
    char    loc[TRACE2_LOC_LEN];
    char    version[2];
    char    datatype[3];
    char    quality[2];
    char    pad[2];
    
  ctypedef struct Tv20:
    char    quality[2];
    char    pad[2];
    
  ctypedef struct Tv21:
    float   conversion_factor;
  
  ctypedef union Tv2:
    Tv20 v20;
    Tv21 v21;

  ctypedef struct TRACE2X_HEADER:
    int     pinno;
    int     nsamp;
    double  starttime;
    double  endtime;
    double  samprate;
    char    sta[TRACE2_STA_LEN];
    char    net[TRACE2_NET_LEN];
    char    chan[TRACE2_CHAN_LEN];
    char    loc[TRACE2_LOC_LEN];
    char    version[2];
    char    datatype[3];
    Tv2     x;

  cdef const int MAX_TRACEBUF_SIZ

  ctypedef union TracePacket:
    char          msg[MAX_TRACEBUF_SIZ];
    TRACE_HEADER  trh;
    TRACE2_HEADER trh2;
    TRACE2X_HEADER trh2x;
    int           i;
