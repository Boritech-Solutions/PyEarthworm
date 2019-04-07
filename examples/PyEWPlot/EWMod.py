#    EWMod uses PyEW to interface a PyEWPlot to the EW Transport system
#    Copyright (C) 2019  Francisco J Hernandez Ramirez
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

from threading import Thread
import PyEW, time, json, datetime
# For Python2
#import matplotlib as mpl
#mpl.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import io

class EWPyPlotter():

  def __init__(self, minutes = 1, defring = 1000, defmod = 9, defins = 141, defhb = 30.0, debug = False):
    
    # Create a thread for self
    self.myThread = Thread(target=self.run)
    
    # Start an EW Module with parent ring 1000, mod_id 8, inst_id 141, heartbeat 30s, debug = False (MODIFY THIS!)
    self.ring2plot = PyEW.EWModule(defring, defmod, defins, defhb, debug)
    
    # Add our Input ring as Ring 0
    self.ring2plot.add_ring(defring)
    
    # Buffer (minutes to buffer for)
    self.minutes = minutes
    self.wave_buffer = {}
    self.time_buffer = {}
    self.chan_buffer = {}
    
    # Allow it to run
    self.runs = True
    self.debug = debug
    
  def save_wave(self):
    
    # Fetch a wave from Ring 0
    wave = self.ring2plot.get_wave(0) 
    
    # if wave is empty return
    if wave == {}: 
      return
    
    # Lets try to buffer with python dictionaries
    name = wave["station"] + '.' + wave["channel"] + '.' + wave["network"] +'.' + wave["location"]
    stat = wave["station"]
    
    # Lets check if we have a buffer for this Station/Channel combo
    if name in self.wave_buffer :
    
        # Determine max samples for buffer
        max_samp = wave["samprate"] * 60 * self.minutes
        
        # Generate the time array
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
            self.wave_buffer[name] = self.wave_buffer[name][tuple([np.s_[samp::]])]
            self.time_buffer[name] = self.time_buffer[name][tuple([np.s_[samp::]])]
            
            # Keep last and Clear image buffer
            self.chan_buffer[name].truncate(0)
            self.chan_buffer[name].seek(0)
            
            ## Generate image and keep it in a memory buffer
            ## We can edit the final figure here:
            gain = 4.6799235e-04
            fsz = 13  ## figure font size
            figx = 15 ## figure size parameter x
            figy = 3  ## figure size parameter y
            mycolors = ["#3F5D7D", "black", "#4169E1"] 
            th = 0.5
            plt.figure(figsize=(figx,figy))
            plt.clf()
            # Alternatives include bmh, fivethirtyeight, ggplot,
            # dark_background, seaborn-deep, etc
            plt.style.use('bmh')
            plt.rcParams['font.family'] = 'sans-serif'
            plt.rcParams['font.serif'] = 'Ubuntu'
            plt.rcParams['font.monospace'] = 'Ubuntu Mono'
            plt.rcParams['font.size'] = fsz
            plt.rcParams['axes.labelsize'] = fsz
            plt.rcParams['axes.labelweight'] = 'bold'
            plt.rcParams['axes.titlesize'] = fsz
            plt.rcParams['xtick.labelsize'] = fsz-1
            plt.rcParams['ytick.labelsize'] = fsz-1
            plt.rcParams['legend.fontsize'] = fsz
            plt.rcParams['figure.titlesize'] = fsz+2		
            plt.plot(self.time_buffer[name], signal.detrend(gain*self.wave_buffer[name]),color=mycolors[1], lw = th)
            plt.gcf().autofmt_xdate()
            plt.title(name,loc='right')
            plt.grid(True)
            plt.gca().spines["top"].set_visible(False)
            plt.gca().spines["right"].set_visible(False)
            plt.gca().spines["bottom"].set_color('grey')
            plt.gca().spines["left"].set_color('grey')
            plt.ylabel('Acc. ($cm/s/s$)',fontsize=fsz)
            plt.savefig(self.chan_buffer[name], format='jpg')
            plt.close()
            
            # Debug data
            if self.debug:
                print("Data was sliced at sample:")
                print(samp)
                print("New Size:")
                print(self.wave_buffer[name].size) 
                print(self.time_buffer[name].size)
    else:
        
        # Generate the time array
        time_array = np.zeros(wave['data'].size)
        time_skip = 1/wave['samprate']
        for i in range(0, wave['data'].size):
            time_array[i] = (wave['startt'] + (time_skip*i)) * 1000
        
        # First instance of data in buffer, create buffer:
        self.wave_buffer[name] = wave["data"]
        self.time_buffer[name] = np.array(time_array, dtype='datetime64[ms]')
        self.chan_buffer[name] = io.BytesIO()
        
        # Debug data
        if self.debug:
            print("First instance of station/channel:")
            print(name)
            print("Size:")
            print(self.wave_buffer[name].size)
            print(self.time_buffer[name].size)
  
  def get_frame(self, station):
    # Function returns a bytearray of the current graph
    if station in self.chan_buffer :
      try:
        val = self.chan_buffer[station].getvalue()
      except OSError as e:
        print(e)
      return val
    else:
      return
      
  def get_menu(self):
    # Function returns a dictionary of station and channels
    return self.chan_buffer.keys()
  
  def status(self):
    return self.runs
    
  def run(self):
    # The main loop
    while self.runs:
      if self.ring2plot.mod_sta() is False:
        break
      time.sleep(0.001)
      self.save_wave()
    self.ring2plot.goodbye()
    print ("Exiting")
      
  def start(self):
    self.myThread.start()
    
  def stop(self):
    self.runs = False
 
