# This is a test module using PyEW Library

import numpy as np
from threading import Thread
from pymongo import MongoClient
import PyEW, time, json, datetime

class Ring2Plot():

  def __init__(self):
    
    # Create a thread for the Module
    self.myThread = Thread(target=self.run)
    self.client = MongoClient()
    
    # Start an EW Module with parent ring 1000, mod_id 8, inst_id 141, heartbeat 30s, debug = False
    self.ring2plot = PyEW.EWModule(1000, 8, 141, 30.0, False) 
    self.client = MongoClient('mongodb://localhost:27017/')
    
    # Add our Input ring as Ring 0
    self.ring2plot.add_ring(1000)
    self.db = self.client['ew-waves']
    
    # Allow it to start
    self.runs = True
    
  def plot_wave(self):
    # Fetch a wave from Ring 0
    wave = self.ring2plot.get_wave(0)
    
    # if wave is empty return
    if wave == {}: 
      return
      
    # Generate the time array 
    time_array = np.array(np.arange(wave['startt'],wave['endt']+1/wave['samprate'],1/wave['samprate'])*1000, dtype='datetime64[ms]')
    
    # Change the time array from numpy to string, data from numpy to string
    # Could use: time_array.tolist() but nodejs is botching the ISOString input
    # Could use:  np.array2string(time_array, max_line_width=4096) but nodejs botches that too
    wave['times'] = time_array.astype('uint64').tolist()
    wave['data']= wave['data'].tolist()
    wave['time'] = datetime.datetime.utcnow()
     
    # Store in mongodb
    if wave['station'] not in self.db.list_collection_names():
      self.db.create_collection(wave['station'], capped=True, size=10000000)
    wave_id = self.db[wave['station']].insert_one(wave).inserted_id
    
    #print(wave_id)
      
  def run(self):
  
    # The main loop
    while self.runs:
      if self.ring2plot.mod_sta() is False:
        break
      time.sleep(0.001)
      self.plot_wave()
    self.ring2plot.goodbye()
    self.client.close()
    quit()
    print ("Exiting")
      
  def start(self):
    self.myThread.start()
    
  def stop(self):
    self.runs = False
    
