# file: kom.h 

cdef extern from "inc/kom.h":
  int    k_open( char * ); 
  int    k_open_as_ini( char * );	
  void   k_treat_as_ini();
  void   k_treat_as_ew();
  int    k_close();
  char  *k_get();
  void   k_dump();
  int    k_err();
  int    k_put( char * );
  int    k_rd();
  int    k_rd_raw( int flag_expansion_level );
  int    k_its( char * );
  char  *k_com();
  char  *k_str();
  double k_val();
  int    k_int();
  long   k_long();
  
  void set_kom_log(void (*par_kom_log_func)(char *) );
