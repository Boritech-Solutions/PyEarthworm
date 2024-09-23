# PyEarthworm

PyEarthworm is a python interface to the Earthworm Messaging Transport system. It seeks to create an easy to use framework to create modern earthworm modules with python. The main class handles the EW module basics such as listening to stop messages and creating heartbeats. You can then use python threads to aquire and insert data from multiple EW Rings. Wave data is already returned as a numpy array so you may use fast c-optimized algorithms with cython. It comes with multiple examples which you may modify in order to speed up development.

## Installation

### Preparation

In order to install PyEarthworm, you need to have first installed Earthworm itself. This module builds successfully on Linux against earthworm-7.10 compiled with `EWBITS=64` and with "`-m64 -fPIC`" added to `GLOBALFLAGS` in `${EW_HOME}/environment/ew_linux.bash`. Your milage may vary for other versions of Earthworm, or on other platforms.

To compile the PyEarthworm C extensions, we need to find the Earthworm `.h` files. We'll look in the following places:

```
${EW_HOME}/include
${EW_HOME}/${EW_VERSION}/include
```

Thus, ensure that the `EW_HOME` and `EW_VERSION` environment variables are set apporpriately.

We will also need the `CFLAGS` you used to compile earthworm with.  The value of `CFLAGS` looks something like this on a Linux system:

```
CFLAGS=-fno-stack-protector -fPIC -Dlinux -D_LINUX -D_INTEL -D_USE_SCHED -D_USE_PTHREADS -D_USE_TERMIOS -I/path/to/ew/includes
```

### Installing via pip

To install via pip, ensure that you have a recent version of pip installed (>=10.0) and do:

```
pip install git+https://github.com/Boritech-Solutions/PyEarthworm
```

### Install via conda

```
conda create --name PyEW
conda activate PyEW
conda install numpy
conda install cython
```

### Download a release and install manually

Recent binary compiled releases can be found [here](
https://github.com/Boritech-Solutions/PyEarthworm/releases/latest) pre-compiled for Linux and Windows intended for python 3.7. These can be placed in the dynload or DLL directory in your python path. 

### Install from source

```
git clone https://github.com/Boritech-Solutions/PyEarthworm
cd PyEarthworm
pip install cython numpy
python setup.py install
```

### Build without installing

```
git clone https://github.com/Boritech-Solutions/PyEarthworm
cd PyEarthworm
pip install cython numpy
python setup.py build_ext -i
```

A file named `PyEW.cpython-36m-x86_64-linux-gnu.so` should have been created.

We can test this works by importing into python:

    $ python
    >>> import PyEW

## Usage:

A Jupyter notebook workshop can be found [here](https://github.com/Fran89/PyEarthworm_Workshop).
The main class for communication with earthworm is PyEW.EWModule. It is a class that takes care of most of what a module should do, and it is described as follows:

#### PyEW.EWModule:
  * **PyEW.EWModule(def_ring, mod_id, inst_id, hb_time, db):**  
  This will initiate a EW Module object with a default ring: def_ring, module id: mod_id, installation id: inst_id, heartbeat interval: hb_time, and debuging set to FALSE (by default). This module will initiate a heartbeat thread that will start by default, it will initate a listener by default to stop if EW emitts a stop message. These listeners and heartbeats will be sent to the default ring! Any message that is sent from this module will also inherit the module id and inst id that was setup here.
  
  * **PyEW.EWModule().goodbye():**  
  This method will begin the mod shutdown if called from somewhere else. It is important to shut down all listener threads before any attempt at shutdown of a program. It will print 'Gracefull Shutdown' when ready to end.
  
  * **PyEW.EWModule().mod_sta():**  
  This method will return a boolean value that reflects if the internal state is ok. Listen to this value to check if there has been a request for the module shutdown.
  
  * **PyEW.EWModule().req_syssta()**  
  This method will request and print the EW System status. Mainly used for testing purposes.
  
  * **PyEW.EWModule().add_ring(ring_id):**  
  This will add a ring to an internal buffer of rings. The ring id: ring_id given will add the listener. For example: Mod.add_ring(1000) will add ring **1000** at location **0**. Should you call this method again (e.g. Mod.add_ring(1005)) will add ring **1005** at location **1** and so on. You can add many rings for multiple inputs and outputs.
  
  * **PyEW.EWModule().get_bytes(buf_ring, msg_type)  
  PyEW.EWModule().get_msg(buf_ring, msg_type):**  
  These two methods will get either a bytestring (which you would have to decode) or a text string (which has been decoded for you) from the memory buffer at location: buf_ring __(ring must have been added from the add_ring() method in order for this to work)__ and from the message type: msg_type. Be warned get_msg will expect a null terminated string. If nothing is found it will return an empty string, otherwise it will return a python string or python bytestring.
  
  * **PyEW.EWModule().put_bytes(buf_ring, msg_type, msg)  
  PyEW.EWModule().put_msg(buf_ring, msg_type, msg):**  
  Likewise these two methods will put either a bytestring or a text string into the memory buffer at location: buf_ring __(ring must have been added from the add_ring() method in order for this to work)__ with message type: msg_type. The lenght of the string is determined by the len() method.
  
  * **PyEW.EWModule().get_wave(buf_ring):**
  This method will attempt to retrive a wave message from the memory buffer at location: buf_ring __(ring must have been added from the add_ring() method in order for this to work)__. If it's successfull it will return a python dictionary with the following wave packet information:  
  
        {
          'station': python.string,
          'network': python.string,
          'channel': python.string,
          'location': python.string,
          'nsamp': python.int,
          'samprate': python.int,
          'startt': python.int,
          'endt': python.int,
          'datatype': python.string,
          'data': numpy.array
        }
        
  * **PyEW.EWModule().put_wave(buf_ring, msg):**
  This method will attempt to insert a wave message into the memory buffer at location: buf_ring __(ring must have been added from the add_ring() method in order for this to work)__. The message must be a python dictionary of the following information:  
  
        {
          'station': python.string, # 4 Sta max 
          'network': python.string, # 2 Net max
          'channel': python.string, # 3 Cha max
          'location': python.string,# 2 Cha max
          'nsamp': python.int,
          'samprate': python.int,
          'startt': python.int,
          'endt': python.int, # This one may be ommited and calculated on the fly.
          'datatype': python.string, # i2, i4, f4, ("f8"?!)
          'data': numpy.array
        }
  It must be stressed that the maximum amount of bytes cannot be more than the one specified in the EW Specification (4096). Additionally, Earthworm does not work well (if at all?) with double precision and extra care must be made when inserting data into EW.
  
 However PyEarthWorm has various classes that have various degrees of abstractions, these are more low level and should not be used unless absolutely needed. For example:

#### PyEW.ring:
  * **PyEW.ring(ring_id):**  
  This will initiate a ring object that can communicate with ring: ring_id.
  
  * **PyEW.ring().attach():**  
  This method will attach the ring.
  
  * **PyEW.ring().detach():**  
  This method will detach the ring.
  
  * **PyEW.ring().get_buffer():**  
  This method will return the pointer to the memory buffer.
  
#### PyEW.transport:
  * **PyEW.transport(ring_id, mod, inst):**  
  This will initiate a transport object with ring: ring_id, module id: mod, and installation id: inst. This object will be automatically attached to the ring.
  
  * **PyEW.transport().detach():**  
  This method detaches the object from the ring. No way to re-attach.
  
  * **PyEW.transport().putmsg(mtype, msg, size):**  
  This method will insert a message into a ring of type: mtype, bytestring: msg, and size: size.
  
  * **PyEW.transport().getmsg_type(mtype):**  
  This method will attempt to get a message from the ring. Returns a tuple of msg.lengh and msg. The tuple will be 0, 0 if unsuccesfull.
  
  * **PyEW.transport().copymsg_type(mtype)**  
  This method will attempt to copy the messagea from the ring. Returns a tuple of msg.lengh and msg. The tuple will be 0, 0 if unsuccesfull.
  
  * **PyEW.transport().reqsta()**  
  This method will return a tuple with the system status.

### Logging
As of May 1, 2019 this module uses python naitive logging levels. In order to see log messages please use:

    import logging
    logging.getLogger().addHandler(logging.StreamHandler())
    logging.getLogger().setLevel(logging.INFO)

You may log to stdout or stderr, or you may add a file handler to be able to see log files from this module.
Initializing with debug = True will add a much more verbose output of log messages and might create big files,
use with caution.
  
## Examples:
  Included with PyEarthworm is a series of examples that may help you in figuring out how this works:
  * The Ring2Ring module is a reimagined in python Ring2Ring module. It has no way to filter, however it can be added to suit your needs. It may have multiple input and output rings and can be used to collapse multiple ring2ring instances.
  * The BNC2Ring module is essentially a NMEAString2EW module that will take input from BNC PPP and place it in a EW Ring.
  * The gsof2Ring module is a modified version of UNAVCO's python script to read gsof and insert them into an EW Ring.
  * The Ring2Mongo module will take wave information and store it in a mongo database (ew-waves). By default it creates a capped collection of 10 Mb for every station regardless of channel. Unless the (time) version is used it will store 3 minutes of data.
  * The Mongo2Ring module can create a listener for changes to an Mongo database that is receiving EW Wave JSON objects. It then modifies these objects in order to be able to add them to a EW Ring. A MongoDB has the advantage of being able to push to multiple listeners and sideways scalability making it ideal to connect an EW to a main database. It requires the latest MongoDB and PyMongo, due to the use of watch pointers.
  * Finally the Ring2Plot is a time limitied Ring2Mongo with a meteor nodejs application (ewrttv) that can be used to plot and display data to a browser (3 components, 1 station). A live version of this can be found [HERE](http://ewrttv.fran89.com). Additionally a single component version is availible in a different branch (singleplot).
  
#### To use a module:
  To start a module you should change directory into the modules main folder and start a python shell:
  
    $ cd example/Ring2Ring/
    $ python
    >>> import EWMod
    >>> Module = EWMod.Ring2Ring()
    >>> Module.start()
  
  To stop the module:
  
    >>> Module.stop()
    
  In order to start and stop it from startstop it must be modified to start from a shell script and then place that shell script in the startstop_\*.d. Additionally a dummy .desc file must be created in the param directory and placed in statmgr.d so that your startstop doesn't get spammed with:
  
    UTC_Thu Jul  5 23:03:35 2018  EW/statmgr msg from unknown module  (statmgr doesn't have a .desc file for this one) inst:141 mod:8 typ:3
  
I hope that you can use these examples to build your own modules that are shared with the community.
  
### Acknowledgments
-------------------

  * I would like to thank ISTI and the EW community, without their contributions to EW this software would not be possible.
  * Chris Malek for making the package installable via PIP.
  * The development and maintenance of PyEarthworm is funded entirely by software and research contracts with Boritech Solutions.
  * The python and cython community.
  
#### AD:
Boritech Solutions is a consulting firm that can help you set up and create modules with PyEarthworm. Contact Us today!
www.BoritechSolutions.com

~Francisco.
  

  

