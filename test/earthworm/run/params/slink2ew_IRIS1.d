#
#                     Configuration File for slink2ew
#
MyModuleId       MOD_SLINK2EW_IRIS1
RingName         WAVE_RING       # Transport ring to write data to.

HeartBeatInterval     30         # Heartbeat interval, in seconds.
LogFile               1          # 1 -> Keep log, 0 -> no log file
                                 # 2 -> write to module log but not stderr/stdout
Verbosity      0		 # Set level of verbosity.

SLhost   rtserve.earthscope.org # Host address of the SeedLink server
SLport         18000             # Port number of the SeedLink server

StateFile     #[filename]        # If this flag is specified (uncommented) a
                                 # file with a list of sequence numbers is
                                 # written, during a clean module shutdown,
                                 # to the parameter directory with the name
                                 # "slink<mod id>.state".  Alternatively, a
                                 # filename may be specified.  During module
                                 # startup these sequence numbers are used to
                                 # resume data streams from the last received
                                 # data.  Using this functionality is highly
                                 # recommended.

#StateFileInt   100              # This controls the interval (in packets
	                         # received) at which the state is saved in
                                 # the state file.  Default is 100 packets,
                                 # 0 to disable.

#NetworkTimeout 600              # Network timeout, after this many idle
                                 # seconds the connection will be reset.
                                 # Default is 600 seconds, 0 to disable.

#NetworkDelay   30               # Network re-connect delay in seconds.

#KeepAlive      0                # Send keepalive packets (when idle) at this
                                 # interval in seconds.  Default is 0 (disabled).

#MaxRate        0                # Limit receive rate to specified bytes/second.
                                 # Default is 0 (disabled, no limit).

#WriteMSEED 0                    # Write miniSEED (TYPE_MSEED) messages to the
                                 # ring instead of tracebufs (TYPE_TRACEBUF2).

#MseedRing  WAVE_RING            # Write miniSEED (TYPE_MSEED) messages to the
                                 # specified ring.  If WriteMSEED not enabled,
                                 # this is in ADDITION to the normal tracebuf
                                 # output; if it is, this will be done INSTEAD of
                                 # writing miniSEED data to the output ring.

instId INST_PRSN                # Override installation ID that is included
                                 # in output messages.  By default this ID is
                                 # determined from the local installation defined
                                 # by the EW_INSTALLATION environment variable.

#SLRecSize       512             # Size (in bytes) of the SEED records expected
                                 # from the server.  Traditionally SeedLink only
                                 # uses 512-byte SEED records.  This option is for
                                 # use with specialized servers that use alternate
                                 # records lengths such as 256 or 128.  One such
                                 # specialized server is the RockToSLink module.

# Selectors and Stream's.  If any Stream lines are specified the connection
# to the SeedLink server will be configured in multi-station mode using
# Selectors, if any, as defaults.  If no Stream lines are specified the
# connection will be configured in uni-station mode using Selectors, if any.

#Selectors      "BH?.D"          # SeedLink selectors.  These selectors are used
                                 # for a uni-station mode connection.  If one
                                 # or more 'Stream' entries are given these are
                                 # used as default selectors for multi-station
                                 # mode data streams.  See description of
                                 # SeedLink selectors below.  Multiple selectors
                                 # must be enclosed in quotes.


# List each data stream (a network and station code pair) that you
# wish to request from the server with a "Stream" command.  If one or
# more Stream commands are given the connection will be configured in
# multi-station mode (multiple station data streams over a single
# network connection).  If no Stream commands are specified the
# connection will be configured in uni-station mode, optionally using
# any specified "Selectors".  A Stream command should be followed by a
# stream key, a network code followed by a station code separated by
# an underscore (i.e. IU_KONO).  SeedLink selectors for a specific
# stream may optionally be specified after the stream key.  Multiple
# selectors must be enclosed in quotes.  Any selectors specified with
# the Selectors command above are used as defaults when no selectors
# are specified for a given stream.

#Stream  GE_DSB   "BH?.D HH?.D"
#Stream  II_KONO  00BH?.D
#
#Stream  IU_ANMO  00BH?.D

#Stream  UW_HOOD
Stream  PR_PRSN
Stream. PR_ICMP

# Some SeedLink servers support extended selection capability and
# allow wildcars (either '*' or '?') for both the network and station
# fields, for example to request all stations from the TA network:

#Stream  TA_*


#(notes regarding "selectors" from a SeedLink configuration file)
#
#   The "selectors" parameter tells to request packets that match given
#   selectors. This helps to reduce network traffic. A packet is sent to
#   client if it matches any positive selector (without leading "!") and
#   doesn't match any negative selectors (with "!"). General format of
#   selectors is LLSSS.T, where LL is location, SSS is channel, and T is
#   type (one of DECOTL for data, event, calibration, blockette, timing,
#   and log records). "LL", ".T", and "LLSSS." can be omitted, meaning
#   "any". It is also possible to use "?" in place of L and S.
#
#   Some examples:
#   BH?            - BHZ, BHN, BHE (all record types)
#   00BH?          - BHZ, BHN, BHE with location code '00' (all record types)
#   BH?.D          - BHZ, BHN, BHE (data records)
#   BH? !E         - BHZ, BHN, BHE (excluding detection records)
#   BH? E          - BHZ, BHN, BHE plus detection records of all channels
#   !LCQ !LEP      - exclude LCQ and LEP channels
#   !L !T          - exclude log and timing records
#
#
# For slink2ew no record types except data records will be written to
# the ring.  In other words, requesting any records in addition to
# data records is a waste.
