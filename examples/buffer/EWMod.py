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

import PyEW
import time
from threading import Thread
import numpy as np

class Ring2Buff():

  def __init__(self, minutes = 1, debug = False):
    
    # Create a thread for self
    self.myThread = Thread(target=self.run)
    
    # Start an EW Module with parent ring 1000, mod_id 8, inst_id 141, heartbeat 30s, debug = False (MODIFY THIS!)
    self.ring2buff = PyEW.EWModule(1000, 8, 141, 30.0, False)
    
    # Add our Input ring as Ring 0
    self.ring2buff.add_ring(1000)
    
    # Buffer (minutes to buffer for)
    self.minutes = minutes
    self.wave_buffer = {}
    self.time_buffer = {}
    
    # Allow it to run
    self.runs = True
    self.debug = debug
    
  def save_wave(self):
    
    # Fetch a wave from Ring 0
    wave = self.ring2buff.get_wave(0) 
    
    # if wave is empty return
    if wave == {}: 
        return
    
    # Lets try to buffer with python dictionaries
    name = wave["station"] +'.' + wave["channel"]
    
    if name in self.wave_buffer :
    
        # Determine max samples for buffer
        max_samp = wave["samprate"] * 60 * self.minutes
        
        # Prep time array
        time_array = np.zeros(wave['data'].size)
        time_skip = 1/wave['samprate']
        for i in range(0, wave['data'].size):
            time_array[i] = (wave['startt'] + (time_skip*i)) * 1000
        time_array = np.array(time_array, dtype='datetime64[ms]')
        
        # Append data to buffer
        self.wave_buffer[name] = np.append(self.wave_buffer[name], wave["data"] )
        self.time_buffer[name] = np.append(self.time_buffer[name], time_array )
        self.time_buffer[name] = self.time_buffer[name][0:self.wave_buffer[name].size]
        
        # Debug data
        if self.debug:
            print("Station Channel combo is in buffer:")
            print(name)
            print("Max:")
            print(max_samp)
            print("Size:")
            print(self.wave_buffer[name].size)
            print(self.time_buffer[name].size)
            
        # If Data is bigger than buffer take a slice
        if self.wave_buffer[name].size >= max_samp:
        
            # Determine where to cut the data and slice
            samp = int(np.floor(self.wave_buffer[name].size - max_samp))
            self.wave_buffer[name] = self.wave_buffer[name][[np.s_[samp::]]]
            self.time_buffer[name] = self.time_buffer[name][[np.s_[samp::]]]
            
            # Debug data
            if self.debug:
                print("Data was sliced at sample:")
                print(samp)
                print("New Size:")
                print(self.wave_buffer[name].size) 
                print(self.time_buffer[name].size)
    else:
    
        # Prep time array
        time_array = np.zeros(wave['data'].size)
        time_skip = 1/wave['samprate']
        for i in range(0, wave['data'].size):
            time_array[i] = (wave['startt'] + (time_skip*i)) * 1000
        time_array = 
        
        # First instance of data in buffer, create buffer:
        self.wave_buffer[name] = wave["data"]
        self.time_buffer[name] = np.array(time_array, dtype='datetime64[ms]')
        self.time_buffer[name] = self.time_buffer[name][0:self.wave_buffer[name].size]
        
        # Debug data
        if self.debug:
            print("First instance of station/channel:")
            print(name)
            print("Size:")
            print(self.wave_buffer[name].size)
            print(self.time_buffer[name].size)
    
  def run(self):
    
    # The main loop
    while self.runs:
      if self.ring2buff.mod_sta() is False:
        break
      time.sleep(0.001)
      self.save_wave()
    self.ring2buff.goodbye()
    print ("Exiting")
      
  def start(self):
    self.myThread.start()
    
  def stop(self):
    self.runs = False
 
