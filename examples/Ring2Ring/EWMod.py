# This is a test module using PyEW Library

import PyEW
import time
from threading import Thread

class Ring2Ring():

  def __init__(self):
    
    # Create a thread for self
    self.myThread = Thread(target=self.run)
    
    # Start an EW Module with parent ring 1000, mod_id 8, inst_id 141, heartbeat 30s, debug = False
    self.ring2ring = PyEW.EWModule(1000, 8, 141, 30.0, False)
    
    # Add our Input ring as Ring 0
    self.ring2ring.add_ring(1000)
    
    # Add our Output ring as Ring 1
    self.ring2ring.add_ring(1005)
    
    # Allow it to run
    self.runs = True
    
  def copy_wave(self):
    
    # Fetch a wave from Ring 0
    wave = self.ring2ring.get_wave(0) 
    
    # if wave is empty return
    if wave == {}: 
      return
    
    # Put into Ring 1 the fetched wave
    self.ring2ring.put_wave(1, wave) 
    
  def run(self):
    
    # The main loop
    while self.runs:
      if self.ring2ring.mod_sta() is False:
        break
      time.sleep(0.001)
      self.copy_wave()
    self.ring2ring.goodbye()
    print ("Exiting")
      
  def start(self):
    self.myThread.start()
    
  def stop(self):
    self.runs = False
 
