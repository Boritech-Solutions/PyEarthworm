#    BNC2Ring is an example of how to use PyEW to interface NMEA messages from BNC to the EW Transport system.
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

import PyEW
import pynmea2
import socket
import argparse
import sys
import numpy as np
import optparse
import time
import datetime
from threading import Thread

# Declare the station to be processed: PRSN - Puerto Rico Seismic Network and its ECEF XYZ position
STA_X=2353900.1799
STA_Y=-5584618.6433
STA_Z=1981221.1234
#

# Transform from Lat Long Height to X, Y, Z
def LLHtoECEF(lat, lon, alt):
  # see http://www.mathworks.de/help/toolbox/aeroblks/llatoecefposition.html
  
  rad = np.float64(6378137.0)        # Radius of the Earth (in meters)
  f = np.float64(1.0/298.257222101)  # Flattening factor GRS80 Model
  cosLat = np.cos(np.deg2rad(lat))
  sinLat = np.sin(np.deg2rad(lat))
  FF     = (1.0-f)**2
  C      = 1/np.sqrt(cosLat**2 + FF * sinLat**2)
  S      = C * FF
  
  x = (rad * C + alt)* cosLat * np.cos(np.deg2rad(lon))
  y = (rad * C + alt)* cosLat * np.sin(np.deg2rad(lon))
  z = (rad * S + alt)* sinLat
  
  return (x, y, z)

class Bnc2Ring():

  def __init__(self, Station = 'PRSN', Network = 'PR', IP = "localhost", PORT = 4000):
  
    # Create a thread for the Module
    self.myThread = Thread(target=self.run)
    
    # Create a socket
    self.sock = socket.socket()
    
    # Connect to BNC NMEA
    self.sock.connect((IP, PORT))
     
    # Start an EW Module with parent ring 1000, mod_id 8, inst_id 141, heartbeat 30s, debug = False (MODIFY THIS!)
    self.bnc2ring = PyEW.EWModule(1000, 8, 141, 30.0, False)
    self.Station = Station
    self.Network = Network
    
    # Remember dtype must be int32
    self.dt = np.dtype(np.int32)
    
    # Add output to Module as Output 0
    self.bnc2ring.add_ring(1000)
    
    # Allow it to start
    self.runs = True
    
  # Read lines from socket
  def readlines(self, sock, recv_buffer=4096, delim='\n'):
    buffer = ''
    data = True
    while data:
      if self.runs is False:
        print "No longer listening to BNC"
        break
      data = sock.recv(recv_buffer)
      buffer += data
      while buffer.find(delim) != -1:
        line, buffer = buffer.split('\n', 1)
        yield line
    return
  
  # Get GPSDATA
  def getGPS(self, line):
    if "GPGGA" in line:
      msg = pynmea2.parse(line)
      xval, yval, zval = LLHtoECEF(msg.latitude, msg.longitude, msg.altitude)
      unixdate = datetime.datetime.utcnow()
      unixtime = datetime.datetime.combine(unixdate.date(),msg.timestamp)
      
      # Convert to Timestamp
      unxtime = int((unixtime - datetime.datetime.utcfromtimestamp(0)).total_seconds())
      
      #print unxtime
      
      # Create EW Wave to send
      xdat = (xval-STA_X)*1000
      X = {
        'station': self.Station,
        'network': self.Network,
        'channel': 'GPX',
        'location': '--',
        'nsamp': 1,
        'samprate': 1,
        'startt': unxtime,
        #'endt': unixtime+1,
        'datatype': 'i4',
        'data': np.array([xdat], dtype=self.dt)
      }
      
      ydat = (yval-STA_Y)*1000
      Y = {
        'station': self.Station,
        'network': self.Network,
        'channel': 'GPY',
        'location': '--',
        'nsamp': 1,
        'samprate': 1,
        'startt': unxtime,
        #'endt': unxtime + 1,
        'datatype': 'i4',
        'data': np.array([ydat], dtype=self.dt)
      }
      
      zdat = (zval-STA_Z)*1000
      Z = {
        'station': self.Station,
        'network': self.Network,
        'channel': 'GPZ',
        'location': '--',
        'nsamp': 1,
        'samprate': 1,
        'startt': unxtime,
        #'endt': unxtime + 1,
        'datatype': 'i4',
        'data': np.array([zdat], dtype=self.dt)
      }
      
      # Send to EW
      self.bnc2ring.put_wave(0, X)
      self.bnc2ring.put_wave(0, Y)
      self.bnc2ring.put_wave(0, Z)
  
  def run(self):
    for line in self.readlines(self.sock):
      time.sleep(0.001)
      if self.runs is False or self.bnc2ring.mod_sta() is False:
        self.runs = False
        break
      self.getGPS(line)
    self.bnc2ring.goodbye()
    self.sock.close()
    quit()
  
  def start(self):
    self.myThread.start()
    
  def stop(self):
    self.runs = False
