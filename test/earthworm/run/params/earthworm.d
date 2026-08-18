#  1999/07/28
#  The working copy of earthworm.d should live in your EW_PARAMS directory.
#
#  An example copy of earthworm.d resides in the vX.XX/environment
#  directory of this Earthworm distribution.

#                       earthworm.d

#              Earthworm administrative setup:
#              Installation-specific info on
#                    shared memory rings
#                    module ids
#                    message types

#   Please read all comments before making changes to this file.
#   The character string <-> numerical value mapping for certain
#   module id's and message types are sacred to earthworm.d
#   and must not be changed!!!

#--------------------------------------------------------------------------
#                      Shared Memory Ring Keys
#
# Define unique keys for shared memory regions (transport rings).
# All string/value mappings for shared memory keys may be locally altered.
#
# The maximum length of ring string is 32 characters.
#--------------------------------------------------------------------------
     
 Ring   WAVE_RING        1000    # public waveform data
# Ring   PICK_RING        1005    # public parametric data
# Ring   HYPO_RING        1015    # public hypocenters etc.
# Ring   BINDER_RING      1020    # private buffer for binder_ew
# Ring   EQALARM_EW_RING  1025    # private buffer for eqalam_ew
# Ring   DRINK_RING       1030    # DST drink messages
# Ring   AD_RING          1035    # A/D waveform ring
# Ring   CUBIC_RING       1036    # private buffer for cubic_msg
 Ring   STATUS_RING      1040    # Ring for status messages
# Do not put FLAG_RING in starstop*.d  This is a hidden ring used for termination flags.
 Ring   FLAG_RING        2000    # a private ring for Startstop 7.5 and later to use
                                 # for flags. If this doesn't exist, startstop will 
                                 # create the ring automatically at key 9999
                                 # If you run multiple startstops, you'll need to change
                                 # all ring keys values here in earthworm.d, including
                                 # this one. Do NOT include this private ring in
                                 # the ring area in startstop*d

 Ring   NAMED_EVENT_RING 2001    # this is a ring used for the tsunami ATWC programs for 
                                 # a named event API (see src/libsrc/util/ew_nevent_message.c)
			    	 # this does not need to be declared, but it is if you wish
				 # to have multiple earthworms running on the same host computer


#--------------------------------------------------------------------------
#                           Module IDs
#
#  Define all module name/module id pairs for this installation
#  Except for MOD_WILDCARD, all string/value mappings for module ids
#  may be locally altered. The character strings themselves may also
#  be changed to be more meaningful for your installation.
#
#  0-255 are the only valid module ids.
#
# The maximum length of the module string is 32 characters.
# 
# This list is in alphabetical order but doesn't need to be. Go ahead and
# add new modules and module IDs at the end.
#--------------------------------------------------------------------------

 Module   MOD_WILDCARD          0   # Sacred wildcard value - DO NOT CHANGE!!!
 Module   MOD_ADSEND            1
 Module   MOD_ADSEND_A          2
 Module   MOD_ADSEND_B          3
 Module   MOD_ADSEND_C          4
 Module   MOD_AD_DEMUX_A        5
 Module   MOD_AD_DEMUX_B        6
 Module   MOD_AEIC2ARC          7
 Module   MOD_ALARM             8
 Module   MOD_ARC2TRIG          9
 Module   MOD_ARCHMAN           10
 Module   MOD_BINDER            11
 Module   MOD_BINDER_EW         12
 Module   MOD_CARLSTATRIG       13
 Module   MOD_CARLSUBTRIG       14
 Module   MOD_COAXTORING_A      15
 Module   MOD_COAXTORING_B      16
 Module   MOD_COMPRESS_UA       17
 Module   MOD_CONFIG_WS         18
 Module   MOD_CSU_TEST          19
 Module   MOD_CUBIC_MSG         20
 Module   MOD_DEBIAS            21
 Module   MOD_DECIMATE          22
 Module   MOD_DECOMPRESS_UA     23
 Module   MOD_DISKMGR           24
 Module   MOD_ELLIPSE2ARC       25
 Module   MOD_EMAIL_SENDER      26
 Module   MOD_EQALARM_EW        27
 Module   MOD_EQASSEMBLE        28
 Module   MOD_EQFILTER          29
 Module   MOD_EQFILTERII        30
 Module   MOD_EQPRELIM          31
 Module   MOD_EQPROC            32
 Module   MOD_EVANSASSOC        33
 Module   MOD_EVANSTRIG         34
 Module   MOD_EVT_DISCR         35
 Module   MOD_EW2FILE           36
 Module   MOD_EW2LISS           37
 Module   MOD_EW2RSAM           38
 Module   MOD_EW2SEISVOLE       39
 Module   MOD_EW2SSAM           40
 Module   MOD_EWINTEGRATE       41
 Module   MOD_EXPORT_ACK        42
 Module   MOD_EXPORT_GENERIC    43
 Module   MOD_EXPORT_GEN_ACTV   44
 Module   MOD_EXPORT_SCN        45
 Module   MOD_EXPORT_SCNL       46
 Module   MOD_EXPORT_SCNL_ACK   47
 Module   MOD_EXPORT_SCN_ACTV   48
 Module   MOD_EXPORT_SCN_PRI    49      
 Module   MOD_FILE2EW           50      
 Module   MOD_FIR               51
 Module   MOD_GAPLIST           52
 Module   MOD_GCF2EW            53
 Module   MOD_GEQPROC           54
 Module   MOD_GETDST            55
 Module   MOD_GETDST2           56
 Module   MOD_GETTERW           57
 Module   MOD_GLASS_EW          58
 Module   MOD_GLOBALPROC        59
 Module   MOD_GMEW              60
 Module   MOD_GRF2EW            61
 Module   MOD_HELI1             62
 Module   MOD_HELI_EWII         64
 Module   MOD_ID                65
 Module   MOD_IMPORT_ACK        66
 Module   MOD_IMPORT_GENERIC    67
 Module   MOD_IMPORT_GEN_PASV   68
 Module   MOD_IMPORT_IDA        69
 Module   MOD_K2ALL             70
 Module   MOD_K2EW              71
 Module   MOD_LATENCY_MON       72
 Module   MOD_LISS2EW           73
 Module   MOD_LOCALMAG          74
 Module   MOD_LPTRIG_A          75
 Module   MOD_LPTRIG_B          76
 Module   MOD_MAG2ORA           77
 Module   MOD_MLSERVER          78
 Module   MOD_MYMODULE          79
 Module   MOD_NANO2TRACE        80
 Module   MOD_NANOBOX           81
 Module   MOD_NAQS2EW           82
 Module   MOD_NAQSSERSDPT       83
 Module   MOD_NAQSSERTG         84
 Module   MOD_NAQSSOH           85
 Module   MOD_NMXPTOOL          86
 Module   MOD_NOMODULE          87
 Module   MOD_NSMP2EW           88
 Module   MOD_ORAREPORT         89
 Module   MOD_ORATRACE_FETCH    90
 Module   MOD_ORATRACE_MRO      91
 Module   MOD_ORATRACE_REQ      92
 Module   MOD_ORA_TRACE_FETCH   93
 Module   MOD_ORA_TRACE_REQ     94
 Module   MOD_PAGERFEEDER       95
 Module   MOD_PICKER_A          96
 Module   MOD_PICKER_B          97
 Module   MOD_PICKWASHER        98
 Module   MOD_PICK_EW           99
 Module   MOD_PICK_RECORDER     100
 Module   MOD_PKFILTER          101
 Module   MOD_PSNADSEND_A       102
 Module   MOD_Q2EW              103
 Module   MOD_Q3302EW           104
 Module   MOD_QDDS_SENDER       105
 Module   MOD_RAYLOC_EW         106
 Module   MOD_RAYPICKER         107
 Module   MOD_RCV_EW            108
 Module   MOD_RCV_NSN7          109
 Module   MOD_REBOOT_MSS_EW     110
 Module   MOD_REDI2EW           111
 Module   MOD_REFTEK2EW         112
 Module   MOD_REPORT            113
 Module   MOD_REPORT_B          114
 Module   MOD_RINGDUP_GENERIC   115
 Module   MOD_RINGDUP_SCN       116
 Module   MOD_RINGTOCOAX        117
 Module   MOD_RINGTOCOAXII      118
 Module   MOD_SAMTAC2EW         119
 Module   MOD_SARAADSEND_A      120
 Module   MOD_SCN2SCNL          121
 Module   MOD_SCNL2SCN          122
 Module   MOD_SCREAM2EW         123
 Module   MOD_SGRAM             124
 Module   MOD_SHAKEMAPFEED      125
 Module   MOD_SLINK2EW          126
 Module   MOD_SM_EW2ORA         127
 Module   MOD_SRPAR2EW          128
 Module   MOD_SRPARXCHEWSEND    129
 Module   MOD_SRUSB2EW          130
 Module   MOD_STARTSTOP         131
 Module   MOD_STATMGR           132
 Module   MOD_STATRIGFILTER     133
 Module   MOD_STATUS            134
 Module   MOD_TANKPLAYER        135
 Module   MOD_TERRA2EW          136
 Module   MOD_TRG_ASSOC         137
 Module   MOD_TRIG2DISK         138
 Module   MOD_TRIGLIST2ORA      139
 Module   MOD_VDL_EW            140
 Module   MOD_WAVESERVER        141
 Module   MOD_WAVESERVERV       142
 Module   MOD_WFTIMEFILTER      143
 Module   MOD_WS2EW_A           144
 Module   MOD_WSV_TEST          145
 Module   MOD_RUN_IMP_WWS       146
 Module   MOD_RUN_WWS           147
 Module   MOD_CODA_AAV          148
 Module   MOD_CODA_DUR          149
 Module   MOD_PYEW              150
 Module   MOD_SLINK2EW_IRIS1    151
 Module   MOD_SLINK2EW_IRIS2    152



#--------------------------------------------------------------------------
#                          Message Types
#
#     !!!  DO NOT USE message types 0 thru 99 in earthworm.d !!!
#
#  Define all message name/message-type pairs for this installation.
#
#  VALID numbers are:
#
# 100-255 Message types 100-255 are defined in each installation's  
#         earthworm.d file, under the control of each Earthworm 
#         installation. These values should be used to label messages
#         which remain internal to an Earthworm system or installation.
#         The character strings themselves should not be changed because 
#         the strings are often hard-coded into the modules.
#         However, the string/value mappings can be locally altered.
#         Any message types for locally-produced code may be defined here.
#              
#
#  OFF-LIMITS numbers:
#
#   0- 99 Message types 0-99 are defined in the file earthworm_global.d.
#         These numbers are reserved by Earthworm Central to label types 
#         of messages which may be exchanged between installations. These 
#         string/value mappings must be global to all Earthworm systems 
#         in order for exchanged messages to be properly interpreted.
#         
# The maximum length of the type string is 32 characters.
#
#--------------------------------------------------------------------------

# Installation-specific message-type mappings (100-255):
 Message  TYPE_SPECTRA       100
 Message  TYPE_QUAKE2K       101
 Message  TYPE_LINK          102
 Message  TYPE_EVENT2K       103
 Message  TYPE_PAGE          104
 Message  TYPE_KILL          105
 Message  TYPE_DSTDRINK      106
 Message  TYPE_RESTART       107
 Message  TYPE_REQSTATUS     108
 Message  TYPE_STATUS        109
 Message  TYPE_EQDELETE      110
 Message  TYPE_EVENT_SCNL    111
 Message  TYPE_RECONFIG      112
 Message  TYPE_STOP          113  # stop a child. same as kill, except statmgr
                          # should not restart it
 Message  TYPE_CANCELEVENT   114  # used by eqassemble
 Message  TYPE_CODA_AAV      115
 Message  TYPE_ACTIVATE_MODULE 117	 #  used by activated scripts
 Message  TYPE_ACTIVATE_COMPLETE 118	# used by activated scripts
 Message  TYPE_THRESH_ALARM  119  # used by ewthresh

 Message TYPE_EVENT_ALARM   198 # used by seisan_report

#   !!!  DO NOT USE message types 0 thru 99 in earthworm.d !!!
