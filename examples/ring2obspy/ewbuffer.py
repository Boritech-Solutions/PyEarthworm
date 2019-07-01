#    ring2buff is an example of how to use PyEW to interface a Python Program to the EW Transport system
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

from logging.handlers import TimedRotatingFileHandler
import configparser, logging, os, time, PyEW, obspy
from obspy.core import Trace, Stats, UTCDateTime
from obspy.realtime import RtTrace
from threading import Thread
import numpy as np

logger = logging.getLogger(__name__)

class Ring2Buff():

  def __init__(self, minutes, RING_ID, MOD_ID, INST_ID, HB, RING_IN, RING_OUT, debug = False):
    
    # Create a thread for self
    self.myThread = Thread(target=self.run)
    
    # Start an EW Module
    self.ring2buff = PyEW.EWModule(RING_ID, MOD_ID, INST_ID, HB, debug)
    
    # Add our Input ring as Ring 0 and output as Ring 1
    self.ring2buff.add_ring(RING_IN)
    self.ring2buff.add_ring(RING_OUT)
    
    # Buffer (minutes to buffer for)
    self.minutes = minutes
    self.wave_buffer = {}
    
    # Allow it to run
    self.runs = True
    self.debug = debug
    
  def save_wave(self):
    
    # Fetch a wave from Ring 0
    wave = self.ring2buff.get_wave(0) 
    
    # if wave is empty return
    if wave == {}: 
        return
    
    # Lets try to buffer with python dictionaries and obspy
    name = wave["station"] + '.' + wave["channel"] + '.' + wave["network"] + '.' + wave["location"]
    
    if name in self.wave_buffer :
    
        # Determine max samples for buffer
        max_samp = wave["samprate"] * 60 * self.minutes
        
        # Create a header:
        wavestats = Stats()
        wavestats.station = wave["station"]
        wavestats.network = wave["network"]
        wavestats.channel = wave["channel"]
        wavestats.location = wave["location"]
        wavestats.sampling_rate = wave["samprate"]
        wavestats.starttime = UTCDateTime(wave['startt'])
        
        # Create a trace
        wavetrace = Trace(header= wavestats)
        wavetrace.data = wave["data"]
        
        # Try to append data to buffer, if gap shutdown.
        try:
            self.wave_buffer[name].append(wavetrace, gap_overlap_check=True)
        except TypeError as err:
            logger.warning(err)
            self.runs = False
        except:
            raise
            self.runs = False
        
        # Debug data
        if self.debug:
            logger.info("Station Channel combo is in buffer:")
            logger.info(name)
            logger.info("Size:")
            logger.info(self.wave_buffer[name].count())
            logger.debug("Data:")
            logger.debug(self.wave_buffer[name])
           
    else:
        # First instance of data in buffer, create a header:
        wavestats = Stats()
        wavestats.station = wave["station"]
        wavestats.network = wave["network"]
        wavestats.channel = wave["channel"]
        wavestats.location = wave["location"]
        wavestats.sampling_rate = wave["samprate"]
        wavestats.starttime = UTCDateTime(wave['startt'])
        
        # Create a trace
        wavetrace = Trace(header=wavestats)
        wavetrace.data = wave["data"]
        
        # Create a RTTrace
        rttrace = RtTrace(int(self.minutes*60))
        self.wave_buffer[name] = rttrace
        
        # Append data
        self.wave_buffer[name].append(wavetrace, gap_overlap_check=True)
        
        # Debug data
        if self.debug:
            logger.info("First instance of station/channel:")
            logger.info(name)
            logger.info("Size:")
            logger.info(self.wave_buffer[name].count())
            logger.debug("Data:")
            logger.debug(self.wave_buffer[name])
  
  def write_wave(self):
    for name in self.wave_buffer.keys():
      wavestat = self.wave_buffer[name].stats
      samprate = wavestat["sampling_rate"]
      while self.wave_buffer[name].count() > samprate:
        data = self.wave_buffer[name].slice(wavestat["starttime"], wavestat["starttime"]+((samprate-1)/samprate))
        self.wave_buffer[name].trim(starttime=(wavestat["starttime"]+ 1.0))
        wave = {
        'station': wavestat["station"],
        'network': wavestat["network"],
        'channel': wavestat["channel"],
        'location': wavestat["location"],
        'nsamp': samprate,
        'samprate': samprate,
        'startt': wavestat["starttime"].__float__(),
        'datatype': 'i4',
        'data': np.array(data.data, dtype=np.int32)
        }
        self.ring2buff.put_wave(1, wave)
    return
    
  def run(self):
    
    # The main loop
    while self.runs:
      if self.ring2buff.mod_sta() is False:
        break
      time.sleep(0.001)
      self.save_wave()
      self.write_wave()
    self.ring2buff.goodbye()
    raise BufferError("Module exit")
    logger.info("Exiting")
      
  def start(self):
    self.myThread.start()
    
  def stop(self):
    self.runs = False
 
