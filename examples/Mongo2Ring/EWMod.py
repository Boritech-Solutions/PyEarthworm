# This is a test module using PyEW Library

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
    
    # Start an EW Module with parent ring 1000, mod_id 8, inst_id 141, heartbeat 30s, debug = False
    self.mongo2ring = PyEW.EWModule(1000, 8, 141, 30.0, False) 
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
          wave = insert_change['fullDocument']
          wf = wave['data']
          dt = np.dtype(wave['datatype'])
          wave['data'] = np.array(wf, dtype=dt)
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
    
