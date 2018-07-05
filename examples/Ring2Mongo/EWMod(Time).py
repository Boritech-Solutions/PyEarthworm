#    Ring2Mongo is an example of how to use PyEW to interface the EW Transport system to MongoDB for transport (Time limit)
#    Copyright (C) 2018  Francisco J Hernandez Ramirez
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

import numpy as np
from threading import Thread
from pymongo import MongoClient
import PyEW, time, json, datetime

class Ring2Mongo():

  def __init__(self):
    
    # Create a thread for the Module
    self.myThread = Thread(target=self.run)
    self.client = MongoClient()
    
    # Start an EW Module with parent ring 1000, mod_id 8, inst_id 141, heartbeat 30s, debug = False (MODIFY THIS!)
    self.ring2mongo = PyEW.EWModule(1000, 8, 141, 30.0, False) 
    self.client = MongoClient('mongodb://localhost:27017/')
    
    # Add our Input ring as Ring 0
    self.ring2mongo.add_ring(1000)
    self.db = self.client['ew-waves']
    
    # Allow it to start
    self.runs = True
    
  def plot_wave(self):
    # Fetch a wave from Ring 0
    wave = self.ring2mongo.get_wave(0)
    
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
    # if wave['station'] not in self.db.list_collection_names():
    self.db[wave['station']].ensure_index("time", expireAfterSeconds=3*60)
    wave_id = self.db[wave['station']].insert_one(wave).inserted_id
    
    #print(wave_id)
      
  def run(self):
  
    # The main loop
    while self.runs:
      if self.ring2mongo.mod_sta() is False:
        break
      time.sleep(0.001)
      self.plot_wave()
    self.ring2mongo.goodbye()
    self.client.close()
    quit()
    print ("Exiting")
      
  def start(self):
    self.myThread.start()
    
  def stop(self):
    self.runs = False
    
    
