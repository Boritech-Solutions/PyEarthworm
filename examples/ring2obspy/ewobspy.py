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
import argparse, configparser, logging, os, time
from ewbuffer import Ring2Buff
from threading import Thread
import numpy as np

# Lets get the parameter file
Config = configparser.ConfigParser()
parser = argparse.ArgumentParser(description='This is a EW & Obspy buffer')

parser.add_argument('-f', action="store", dest="ConfFile",   default="ewobspy.d", type=str)

results = parser.parse_args()
Config.read(results.ConfFile)

# Setup the module logfile
log_path = os.environ['EW_LOG']
log_name = results.ConfFile.split(".")[0] + ".log"
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
fh = TimedRotatingFileHandler(filename=log_path + log_name, when='midnight', interval=1, backupCount=3)
fh.setLevel(logging.DEBUG)
fh.setFormatter(formatter)
logging.getLogger().addHandler(fh)
logging.getLogger().setLevel(logging.DEBUG)
logger=logging.getLogger('ewalign')

# Config and start module
buffertime = int(Config.get('Buffer','Time'))
main_ring = int(Config.get('Earthworm','RING_ID'))
module_id = int(Config.get('Earthworm','MOD_ID'))
install_id = int(Config.get('Earthworm','INST_ID'))
heartbeat = int(Config.get('Earthworm','HB'))
in_ring = int(Config.get('Rings','IN_RING'))
out_ring = int(Config.get('Rings','OUT_RING'))

logger.info("Starting with buffertime: %i", buffertime)
logger.info("Main ring: %i", main_ring)
logger.info("Module id: %i", module_id)
logger.info("Installation id: %i", install_id)
logger.info("Heartbeat time: %i", heartbeat)
logger.info("Input ring: %i", in_ring)
logger.info("Output ring: %i", out_ring)
logger.info("Config file read, module init")

Buffer = Ring2Buff(buffertime, main_ring, module_id, install_id, heartbeat, in_ring, out_ring, False)

# Main program start
if __name__ == '__main__':
  try:
    logger.info("STATUS: Module starting")
    Buffer.start()
  except BufferError:
    logger.info("STATUS: Module Stopping")
  except KeyboardInterrupt:
    logger.info("STATUS: Module Stopping, you hit ctl+C. ")
    Buffer.stop()
