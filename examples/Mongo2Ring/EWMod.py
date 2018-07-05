#    Mongo2Ring is an example of how to use PyEW to interface the MongoDB to the EW Transport system for transport
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

import numpy as np
from threading import Thread
from pymongo import MongoClient
import pymongo as pymg
import PyEW, time, json, datetime

class Mongo2Ring():

  def __init__(self):
    
    # Create a thread for the Module
    self.myThread = Thread(target=self.run)
    self.client = MongoClient()
    
    # Start an EW Module with parent ring 1000, mod_id 8, inst_id 141, heartbeat 30s, debug = False (MODIFY THIS!)
    self.mongo2ring = PyEW.EWModule(1000, 9, 141, 30.0, False) 
    self.client = MongoClient('mongodb://localhost:27017/')
    
    # Add our Output ring as Ring 0
    self.mongo2ring.add_ring(1005)
    self.db = self.client['ew-waves']
    
    # Allow it to start
    self.runs = True
      
  def run(self):
    # The main loop
    try:
      with self.db.watch([{'$match': {'operationType': 'insert'}}]) as stream:
        for insert_change in stream:
          if self.mongo2ring.mod_sta() is False or self.runs is  False:
            break
          ## Get the wave
          wave = insert_change['fullDocument']
          
          ## Change the data from list to numpy array
          wf = wave['data']
          dt = np.dtype(wave['datatype'])
          wave['data'] = np.array(wf, dtype=dt)
          
          ## Insert wave in to ring
          self.mongo2ring.put_wave(0, wave)
          
    except pymg.errors.PyMongoError:
      # The ChangeStream encountered an unrecoverable error or the
      # resume attempt failed to recreate the cursor.
      print ('Error getting data')
    
    self.mongo2ring.goodbye()
    self.client.close()
    quit()
    print ("Exiting")
      
  def start(self):
    self.myThread.start()
    
  def stop(self):
    self.runs = False
    
