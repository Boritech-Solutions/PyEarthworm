# file: transport.h

cdef extern from "inc/transport.h":
  ctypedef struct SHM_INFO:
    pass

  ctypedef struct MSG_LOGO:
    unsigned char type
    unsigned char mod
    unsigned char instid

  cdef int PUT_OK
  cdef int PUT_NOTRACK
  cdef int PUT_TOOBIG
  cdef int GET_OK
  cdef int GET_NONE
  cdef int GET_MISS
  cdef int GET_NOTRACK
  cdef int GET_TOOBIG
  cdef int GET_MISS_LAPPED
  cdef int GET_MISS_SEQGAP
  cdef int TERMINATE

  void  tport_attach( SHM_INFO *, long );
  void  tport_detach( SHM_INFO * );
  int   tport_putmsg( SHM_INFO *, MSG_LOGO *, long, char * );
  int   tport_getmsg( SHM_INFO *, MSG_LOGO *, short, MSG_LOGO *, long *, char *, long );
  int  tport_getflag( SHM_INFO *);
  int tport_copyto  ( SHM_INFO *, MSG_LOGO *, long , char *, unsigned char);
  int tport_copyfrom( SHM_INFO *, MSG_LOGO *, short, MSG_LOGO *, long *, char *, long, unsigned char *);
