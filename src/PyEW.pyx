#    PyEW is a library that creates a python interface to the Earthworm Transport system
#    Copyright (C) 2018  Francisco J Hernandez Ramirez
#    You may contact me at FJHernandez89@gmail.com, FHernandez@boritechsolutions.com
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>

# distutils: sources = src/transport.c src/sleep_ew.c src/getutil.c src/kom.c src/logit.c src/time_ew.c
# distutils: include_dirs = inc/
    
import os, time, threading
from libc.string cimport memcpy, memset, strncpy
import numpy as np
import struct

cimport ctransport
cimport ctracebuf

cdef extern from "Python.h":
  bytes PyBytes_FromStringAndSize(char *v, Py_ssize_t len)
  
## For eventual move to cython numpy
#cimport numpy as np
#ITYPE = np.int
#DTYPE = np.doubl
#ctypedef np.int_t ITYPE_t
#ctypedef np.double_t DTYPE_t
##

cdef class ring:
  """Ring class allows easy attachment and detachment from a EW Ring"""
  cdef ctransport.SHM_INFO shm_info
  cdef int ring_id
  cdef bint attach_status
  
  def __init__(self, ringid):
    self.ring_id = ringid
    self.attach_status = False
    
  def attach(self):
    if self.ring_id >= 1000:
      ctransport.tport_attach(&self.shm_info, self.ring_id)
      self.attach_status = True
      print "Attached to ring"
    else:
      print "Could not attach to ring"
  
  def detach(self):
    if self.attach_status == True:
      ctransport.tport_detach(&self.shm_info)
      self.attach_status = False 
      print "Detached from ring"
    else:
      print "Could not detach from ring"
  
  cdef ctransport.SHM_INFO* get_buffer(self):
    if self.attach_status == True:
     return &self.shm_info
    else:
      print "Ring not attached"

cdef class transport:
  """ This python class wraps the EW Transport protocol."""
  cdef ring myring
  cdef int mod_id
  cdef int inst_id
    
  def __init__(self, ringid, mod, inst):
    self.myring = ring(ringid)
    self.mod_id = mod
    self.inst_id = inst
    self.myring.attach()
    
  def detach(self):
    self.myring.detach()
    
  def flush(self):
    cdef ctransport.MSG_LOGO reqmsg
    cdef ctransport.MSG_LOGO resp
    cdef char[4096] msg
    cdef long rlen
    cdef int status
    reqmsg.type = 0
    reqmsg.mod = 0
    reqmsg.instid = 0
    while True:
      status = ctransport.tport_getmsg(self.myring.get_buffer(), &reqmsg, 1, &resp, &rlen, msg, 4096)
      if status == ctransport.GET_NONE:
        break
    
  def putmsg(self, mtype, msg, size):
    cdef ctransport.MSG_LOGO msglogo
    msglogo.type = mtype
    msglogo.mod = self.mod_id
    msglogo.instid = self.inst_id
    cdef char* ms = msg
    cdef long sz = size
    ctransport.tport_putmsg(self.myring.get_buffer(), &msglogo, sz, ms)
    
  def getmsg_type(self, mtype):
    cdef ctransport.MSG_LOGO reqmsg
    cdef ctransport.MSG_LOGO resp
    cdef char[4096] msg
    cdef long rlen
    cdef int status
    reqmsg.type = mtype
    reqmsg.mod = 0
    reqmsg.instid = self.inst_id
    status = ctransport.tport_getmsg(self.myring.get_buffer(), &reqmsg, 1, &resp, &rlen, msg, 4096)
    cdef bytes realmsg = PyBytes_FromStringAndSize(msg, 4096)
    if status != ctransport.GET_NONE: # Maybe implement a check for other status
      return (rlen, realmsg)
    else:
      return (0,0)
  
  def copymsg_type(self, mtype):
    cdef ctransport.MSG_LOGO reqmsg
    cdef ctransport.MSG_LOGO resp
    cdef char msg[4096]
    cdef unsigned char seq
    cdef long rlen
    cdef int status
    reqmsg.type = mtype
    reqmsg.mod = 0
    reqmsg.instid = self.inst_id
    status = ctransport.tport_copyfrom(self.myring.get_buffer(), &reqmsg, 1, &resp, &rlen, msg, 4096, &seq)
    cdef bytes realmsg = PyBytes_FromStringAndSize(msg, 4096)
    if status != ctransport.GET_NONE: # Maybe implement a check for other status
      return (status, rlen, realmsg)
    else:
      return (0,0)
  
  def reqsta(self):
    cdef char* message 
    message = '0\n'
    self.putmsg(108, message, len(message))
    time.sleep(5)
    return self.getmsg_type(109)

class heartbeatTimer(threading.Thread):
  """Heartbeat Timer class"""
  def __init__(self):
    threading.Thread.__init__(self)
    
  def setup (self, mtime, mfunct):
    self.time = mtime
    self.funct = mfunct
    self.runs = True
  
  def run(self):
    while self.runs:
      self.funct()
      time.sleep(self.time)
    print("Heartbeat thread successfully ended")
    
  def stop(self):
    print("Heartbeat shutdown requested")
    self.runs = False
    
class stopThread(threading.Thread):
  """stopThread class"""
  def __init__(self):
    threading.Thread.__init__(self)
    
  def setup (self, ringid, modid, instid, mfunct):
    self.temp = transport(ringid, modid, instid)
    self.funct = mfunct
    self.runs = True
  
  def run(self):
    self.temp.flush()
    while self.runs:
      time.sleep(0.1)
      inp = self.temp.getmsg_type(113)
      
      if inp != (0,0):
        pid = inp[1][:inp[0]].decode('UTF-8')
        if pid == str(os.getpid()):
          self.temp.detach()
          self.funct()
        else:
          print "Not my pid"
    print("Stop thread successfully ended")
    
  def stop(self):
    print("Heartbeat shutdown requested")
    self.runs = False
    
class restartThread(threading.Thread):
  """restartThread class"""
  def __init__(self):
    threading.Thread.__init__(self)
    
  def setup (self, ringid, modid, instid, mfunct):
    self.temp = transport(ringid, modid, instid)
    self.funct = mfunct
    self.runs = True
  
  def run(self):
    self.temp.flush()
    while self.runs:
      time.sleep(0.1)
      inp = self.temp.getmsg_type(107)
      
      if inp != (0,0):
        pid = inp[1][:inp[0]].decode('UTF-8')
        if pid == str(os.getpid()):
          self.temp.detach()
          self.funct()
        else:
          print "Not my pid"
    print("Restart thread successfully ended")
    
  def stop(self):
    print("Ring thread shutdown requested")
    self.runs = False

cdef class EWModule:
  """This python class creates an EW Module"""
  cdef int my_ring
  cdef int my_modid
  cdef int my_instid
  cdef int hb
  cdef transport default_ring
  cdef bint debug
  cdef bint OK
  HBT = heartbeatTimer()
  RNG = stopThread()
  RTH = restartThread()
  ringcom = []
  
  def __init__(self, def_ring, mod_id, inst_id, hb_time, db = False):
    print "Module initiated"
    self.my_ring = def_ring
    self.my_modid = mod_id
    self.my_instid = inst_id
    self.hb = hb_time
    self.debug = db
    self.default_ring=transport(self.my_ring, self.my_modid, self.my_instid)
    self.HBT.setup(hb_time, self.send_hb)
    self.HBT.start()
    self.RNG.setup(self.my_ring, self.my_modid, self.my_instid, self.goodbye)
    self.RNG.start()
    self.RTH.setup(self.my_ring, self.my_modid, self.my_instid, self.goodbye)
    self.RTH.start()
    self.OK = True
    
  def send_hb(self):
    msg = str(int(round(time.time()))) + ' ' + str(os.getpid())
    mymsg = msg.encode('UTF-8')
    cdef char* message = mymsg
    if self.OK:
      self.default_ring.putmsg(3, message, len(message))
  
  def goodbye(self):
    if self.OK:
      self.OK = False
      self.HBT.stop()
      self.RNG.stop()
      self.RTH.stop()
      self.default_ring.detach()
      for ring in self.ringcom:
        ring.detach()
      print("Graceful Shutdown")
      time.sleep(self.hb)
      quit()
  
  def mod_sta(self):
    return self.OK
  
  def req_syssta(self):
    if self.OK:
      print "Requesting system status"
      msg = self.default_ring.reqsta()
      status = msg[1][:msg[0]].decode('UTF-8')
      print status
  
  def add_ring(self, ring_id):
    if self.OK:
      print "Add ring to array"
      temp = transport(ring_id, self.my_modid, self.my_instid)
      temp.flush()
      self.ringcom.append(temp)
  
  def get_bytes(self, buf_ring, msg_type):
    if self.debug:
      print "Get msg from array"
    if buf_ring < len(self.ringcom) and self.OK:
      msg = self.ringcom[buf_ring].copymsg_type(msg_type)
      if msg != (0,0):
        if self.debug:
          status = msg[2][:msg[1]].decode('UTF-8')
          print status
      return msg
    return ''
  
  def get_msg(self, buf_ring, msg_type):
    if self.debug:
      print "Get msg from array"
    if buf_ring < len(self.ringcom) and self.OK:
      msg = self.ringcom[buf_ring].copymsg_type(msg_type)
      if msg != (0,0):
        if self.debug:
          status = msg[2][:msg[1]].decode('UTF-8')
          print status
      return status
    return ''
  
  def put_bytes(self, buf_ring, msg_type, msg):
    if self.debug:
      print "Put msg into array"
    if buf_ring < len(self.ringcom) and self.OK:
      self.ringcom[buf_ring].putmsg(msg_type, msg, len(msg))
  
  def put_msg(self, buf_ring, msg_type, msg):
    if self.debug:
      print "Put msg into array"
    if buf_ring < len(self.ringcom) and self.OK:
      self.ringcom[buf_ring].putmsg(msg_type, msg.encode('UTF-8'), len(msg.encode('UTF-8')))
  
  def get_wave(self, buf_ring):
    if self.debug:
      print "Get wave from array"   
    # Info data structs
    cdef ctracebuf.TracePacket mypkt
    cdef char* mymsg
    cdef char* pkt
    
    if buf_ring < len(self.ringcom) and self.OK:
      msg = self.ringcom[buf_ring].copymsg_type(19)
      if msg != (0,0):
        mymsg = msg[2]
        memcpy(&mypkt, mymsg, msg[1])
        
        # myarr = np.array([]) JIC
        
        datatype = mypkt.trh2.datatype.decode('UTF-8')
        if datatype == 'i2':
          mydata = msg[2][sizeof(ctracebuf.TRACE2_HEADER):(sizeof(ctracebuf.TRACE2_HEADER)+mypkt.trh2.nsamp*2)]
          myarr = np.frombuffer(mydata, dtype=np.dtype(mypkt.trh2.datatype.decode('UTF-8')))
        if datatype == 'i4':
          mydata = msg[2][sizeof(ctracebuf.TRACE2_HEADER):(sizeof(ctracebuf.TRACE2_HEADER)+mypkt.trh2.nsamp*4)]
          myarr = np.frombuffer(mydata, dtype=np.dtype(mypkt.trh2.datatype.decode('UTF-8')))
        if datatype == 'i8':
          mydata = msg[2][sizeof(ctracebuf.TRACE2_HEADER):(sizeof(ctracebuf.TRACE2_HEADER)+mypkt.trh2.nsamp*8)]
          myarr = np.frombuffer(mydata, dtype=np.dtype(mypkt.trh2.datatype.decode('UTF-8')))
        if datatype == 's4':
          mydata = msg[2][sizeof(ctracebuf.TRACE2_HEADER):(sizeof(ctracebuf.TRACE2_HEADER)+(int(struct.unpack("<i", struct.pack(">i", mypkt.trh2.nsamp))[0])*4))]
          myarr = np.frombuffer(mydata, dtype=np.dtype('>i4'))
          
        data = {
        'station': mypkt.trh2.sta.decode('UTF-8'),
        'network': mypkt.trh2.net.decode('UTF-8'),
        'channel': mypkt.trh2.chan.decode('UTF-8'),
        'location': mypkt.trh2.loc.decode('UTF-8'),
        'nsamp': mypkt.trh2.nsamp,
        'samprate': mypkt.trh2.samprate,
        'startt': mypkt.trh2.starttime,
        'endt': mypkt.trh2.endtime,
        'datatype': mypkt.trh2.datatype.decode('UTF-8'),
        'data': myarr}
        
        if datatype == 's4':
          data = {
          'station': mypkt.trh2.sta.decode('UTF-8'),
          'network': mypkt.trh2.net.decode('UTF-8'),
          'channel': mypkt.trh2.chan.decode('UTF-8'),
          'location': mypkt.trh2.loc.decode('UTF-8'),
          'nsamp': int(struct.unpack("<i", struct.pack(">i", mypkt.trh2.nsamp))[0]),
          'samprate': struct.unpack("<d", struct.pack(">d", mypkt.trh2.samprate))[0],
          'startt': struct.unpack("<d", struct.pack(">d", mypkt.trh2.starttime))[0],
          'endt': struct.unpack("<d", struct.pack(">d", mypkt.trh2.endtime))[0],
          'datatype': mypkt.trh2.datatype.decode('UTF-8'),
          'data': myarr}
        
        return data
      else:
        return {}
    return {}
  
  def put_wave(self, buf_ring, msg):
    if self.debug:
      print "Put wave into array"
    cdef ctracebuf.TracePacket mypkt
    cdef char *mydata
    cdef char *pkt
    memset(&mypkt,0,sizeof(ctracebuf.TracePacket))
    
    # Set station
    if 'station' in msg:
      strncpy(mypkt.trh2.sta, msg['station'].encode('UTF-8'), ctracebuf.TRACE2_STA_LEN-1)
      mypkt.trh2.sta[ctracebuf.TRACE2_STA_LEN-1] = '\0'
    else:
      if self.debug:
        print "Station is not in trace" 
      return
      
    # Set network
    if 'network' in msg:
      strncpy(mypkt.trh2.net, msg['network'].encode('UTF-8'), ctracebuf.TRACE2_NET_LEN-1)
      mypkt.trh2.net[ctracebuf.TRACE2_NET_LEN-1] = '\0'
    else:
      if self.debug:
        print "Network not in trace"
      return
    
    # Set channel  
    if 'channel' in msg:
      strncpy(mypkt.trh2.chan, msg['channel'].encode('UTF-8'), ctracebuf.TRACE2_CHAN_LEN-1)
      mypkt.trh2.chan[ctracebuf.TRACE2_CHAN_LEN-1] = '\0'
    else:
      if self.debug:
        print "Channel name not in trace"
      return
    
    # Set location
    if 'location' in msg:
      strncpy(mypkt.trh2.loc, msg['location'].encode('UTF-8'), ctracebuf.TRACE2_LOC_LEN-1)
      mypkt.trh2.loc[ctracebuf.TRACE2_LOC_LEN-1] = '\0'
    else:
      if self.debug:
        print "Location not in trace"
      return
    
    # Set datatype
    if 'datatype' in msg:
      strncpy(mypkt.trh2.datatype,msg['datatype'].encode('UTF-8'), 2)
      mypkt.trh2.datatype[2] = '\0'
    else:
      if self.debug:
        print "Data type not in trace"
      return
    
    # Set # of samps
    if 'nsamp' in msg:
      mypkt.trh2.nsamp = msg['nsamp']
      if msg['datatype'] == 's4':
        mypkt.trh2.nsamp = struct.unpack(">i", struct.pack("<i", msg['nsamp']))[0]
    else:
      if self.debug:
        print "Number of samples not in trace"
      return
    
    # Set samp rate
    if 'samprate' in msg:
      mypkt.trh2.samprate = msg['samprate']
      if msg['datatype'] == 's4':
        mypkt.trh2.samprate = struct.unpack(">d", struct.pack("<d", msg['samprate']))[0]
    else:
      if self.debug:
        print "Sample rate not in trace"
      return
    
    # Set start time
    if 'startt' in msg:
      mypkt.trh2.starttime = msg['startt']
      if msg['datatype'] == 's4':
        mypkt.trh2.starttime = struct.unpack(">d", struct.pack("<d", msg['startt']))[0]
    else:
      if self.debug:
        print "Start time not in trace"
      return
    
    # Set end time
    if 'endt' in msg:
      mypkt.trh2.endtime = msg['endt']
      if msg['datatype'] == 's4':
        mypkt.trh2.endtime = struct.unpack(">d", struct.pack("<d", msg['endt']))[0]
    else:
      endtime = msg['startt'] + (( msg['nsamp'] - 1 ) *( 1/msg['samprate'] ))
      mypkt.trh2.endtime = msg['startt'] + ((msg['nsamp'] - 1)/msg['samprate'])
      if msg['datatype'] == 's4':
        mypkt.trh2.endtime = struct.unpack(">d", struct.pack("<d", endtime))[0]
      if self.debug:
        print "End time not in trace, but continuing"
    
    # Set payload
    if 'data' in msg:
      mytemp = msg['data'].tobytes('C')
      mydata = mytemp
      memcpy(&mypkt.msg[sizeof(ctracebuf.TRACE2_HEADER)], mydata, len(mytemp))
    else:
      if self.debug:
        print "No data in trace"
      return
    
    if msg['datatype'] == 'i2':
      length = sizeof(ctracebuf.TRACE2_HEADER)+ (mypkt.trh2.nsamp * 2)
    if msg['datatype'] == 'i4':
      length = sizeof(ctracebuf.TRACE2_HEADER)+ (mypkt.trh2.nsamp * 4)
    if msg['datatype'] == 'f4':
      length = sizeof(ctracebuf.TRACE2_HEADER)+ (mypkt.trh2.nsamp * 4)
    if msg['datatype'] == 'f8':
      length = sizeof(ctracebuf.TRACE2_HEADER)+ (mypkt.trh2.nsamp * 8)
    if msg['datatype'] == 's4':
      length = sizeof(ctracebuf.TRACE2_HEADER)+ (msg['nsamp'] * 4)
    
    pkt = <char*> &mypkt
    
    if self.OK:
      self.ringcom[buf_ring].putmsg(19, pkt[:length], length)

