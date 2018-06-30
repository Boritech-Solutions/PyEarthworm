#!/usr/bin/env python
import PyEW
import pynmea2
import socket
import argparse
import sys
import numpy as np
import optparse
import time
import datetime

def readlines(sock, recv_buffer=4096, delim='\n'):
  buffer = ''
  data = True
  while data:
    data = sock.recv(recv_buffer)
    buffer += data
    while buffer.find(delim) != -1:
      line, buffer = buffer.split('\n', 1)
      yield line
  return
  
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


PRSN_X=2353900.1799
PRSN_Y=-5584618.6433
PRSN_Z=1981221.1234

quitting = False
sock = socket.socket()

def main():
    parser = argparse.ArgumentParser(description='This is a BNC NMEA message parser')
    parser.add_argument('-i', action="store", dest="IP",   default="localhost",   type=str)
    parser.add_argument('-p', action="store", dest="PORT", default=4000,          type=int)
    parser.add_argument('-r', action="store", dest="RING", default=1000,          type=int)
    parser.add_argument('-m', action="store", dest="MODID",default=8,             type=int)
    
    results = parser.parse_args()
    
    # Connect to BNC NMEA
    sock.connect((results.IP, results.PORT))
     
    # Connect to EW
    Mod = PyEW.EWModule(results.RING, results.MODID, 141, 30.0, False)
    Station = 'PRSN'
    Network = 'PR'
    
    # Remember dtype must be int32
    dt = np.dtype(np.int32)
    
    # Add output to Module as Output 0
    Mod.add_ring(results.RING)
    
    for line in readlines(sock):
      time.sleep(0.001)
      if quitting:
        print 'goodbye'
        Mod.goodbye()
        break
      # Read NMEA msg:
      if "GPGGA" in line:
        msg = pynmea2.parse(line)
        xval, yval, zval = LLHtoECEF(msg.latitude, msg.longitude, msg.altitude)
        unixdate = datetime.datetime.utcnow()
        unixtime = datetime.datetime.combine(unixdate.date(),msg.timestamp)
        
        # Convert to Timestamp once more I guess
        unxtime = int((unixtime - datetime.datetime.utcfromtimestamp(0)).total_seconds())
        
        #print unxtime
        
        # Create EW Wave to send
        xdat = (xval-PRSN_X)*1000
        X = {
          'station': Station,
          'network': Network,
          'channel': 'GPX',
          'location': '--',
          'nsamp': 1,
          'samprate': 1,
          'startt': unxtime,
          #'endt': unixtime+1,
          'datatype': 'i4',
          'data': np.array([xdat], dtype=dt)
        }
        
        ydat = (yval-PRSN_Y)*1000
        Y = {
          'station': Station,
          'network': Network,
          'channel': 'GPY',
          'location': '--',
          'nsamp': 1,
          'samprate': 1,
          'startt': unxtime,
          #'endt': unxtime + 1,
          'datatype': 'i4',
          'data': np.array([ydat], dtype=dt)
        }
        
        zdat = (zval-PRSN_Z)*1000
        Z = {
          'station': Station,
          'network': Network,
          'channel': 'GPZ',
          'location': '--',
          'nsamp': 1,
          'samprate': 1,
          'startt': unxtime,
          #'endt': unxtime + 1,
          'datatype': 'i4',
          'data': np.array([zdat], dtype=dt)
        }
        
        # Send to EW
        Mod.put_wave(0, X)
        Mod.put_wave(0, Y)
        Mod.put_wave(0, Z)
        

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        #outfile.close
        sock.close()
        quitting = True
        print("\nSTATUS: Stopping, you hit ctl+C. ")
