from struct import unpack
import socket
import math
import argparse
import json

class Gsof(object):
    
    def __init__(self, sock=None):
        if sock is None:
            self.sock = socket.socket(
                socket.AF_INET, socket.SOCK_STREAM)
        else:
            self.sock = sock
        self.msg_dict = {}
        self.msg_bytes = None
        self.checksum = None
        self.rec_dict = {}

    def connect(self, host, port):
        self.sock.connect((host, port))

    def get_message_header(self):
        data = self.sock.recv(7)
        msg_field_names = (’STX’, ’STATUS’, ’TYPE’, ’LENGTH’,
                           ’T_NUM’, ’PAGE_INDEX’, ’MAX_PAGE_INDEX’)
        self.msg_dict = dict(zip(msg_field_names, unpack(’>7B’, data)))
        self.msg_bytes = self.sock.recv(self.msg_dict[’LENGTH’] - 3)
        (checksum, etx) = unpack(’>2B’, self.sock.recv(2))

        def checksum256(st):
            """Calculate checksum"""
            return reduce(lambda x, y: x+y, map(ord, st)) % 256
        if checksum-checksum256(self.msg_bytes+data[1:]) == 0:
            self.checksum = True
        else:
            self.checksum = False

    def get_records(self):
        while len(self.msg_bytes) > 0:
            # READ THE FIRST TWO BYTES FROM RECORD HEADER
            record_type, record_length = unpack(’>2B’, self.msg_bytes[0:2])
            self.msg_bytes = self.msg_bytes[2:]
            self.select_record(record_type, record_length)
    
    def select_record(self, record_type, record_length):
        if record_type == 1:
            rec_field_names = (’GPS_TIME’, ’GPS_WEEK’, ’SVN_NUM’,
                               ’FLAG_1’, ’FLAG_2’, ’INIT_NUM’)
            rec_values = unpack(’>LH4B’, self.msg_bytes[0:record_length])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 2:
            rec_field_names = (’LATITUDE’, ’LONGITUDE’, ’HEIGHT’)
            rec_values = unpack(’>3d’, self.msg_bytes[0:record_length])
            rec_values = list(rec_values)
            rec_values = (math.degrees(rec_values[0]), math.degrees(rec_values[1]), rec_values[2])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 3:
            rec_field_names = (’X_POS’, ’Y_POS’, ’Z_POS’)
            rec_values = unpack(’>3d’, self.msg_bytes[0:record_length])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 4:
            rec_field_names = (’LOCAL_DATUM_ID’, ’LOCAL_DATUM_LAT’,
                               ’LOCAL_DATUM_LON’, ’LOCAL_DATUM_HEIGHT’, ’OPRT’)
            rec_values = unpack(’>8s3dB’, self.msg_bytes[0:record_length])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 5:
            rec_field_names = (’LOCAL_DATUM_ID’, ’LOCAL_ZONE_ID’,
                               ’LOCAL_ZONE_NORTH’, ’LOCAL_ZONE_EAST’, ’LOCAL_DATUM_HEIGHT’)
            rec_values = unpack(’>2s3d’, self.msg_bytes[0:record_length])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 6:
            rec_field_names = (’DELTA_X’, ’DELTA_Y’, ’DELTA_Z’)
            rec_values = unpack(’>3d’, self.msg_bytes[0:record_length])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 7:
            rec_field_names = (’DELTA_EAST’, ’DELTA_NORTH’, ’DELTA_UP’)
            rec_values = unpack(’>3d’, self.msg_bytes[0:record_length])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 8:
            rec_field_names = (’VEL_FLAG’, ’VELOCITY’, ’HEADING’, ’VERT_VELOCITY’)
            rec_values = unpack(’>B3f’, self.msg_bytes[0:record_length])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 9:
            rec_field_names = (’PDOP’, ’HDOP’, ’VDOP’, ’TDOP’)
            rec_values = unpack(’>4f’, self.msg_bytes[0:record_length])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 10:
            rec_field_names = (’CLOCK_FLAG’, ’CLOCK_OFFSET’, ’FREQ_OFFSET’)
            rec_values = unpack(’>B2d’, self.msg_bytes[0:record_length])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 11:
            rec_field_names = (’POSITION_RMS_VCV’, ’VCV_XX’, ’VCV_XY’, ’VCV_XZ’,
                               ’VCV_YY’, ’VCV_YZ’, ’VCV_ZZ’, ’UNIT_VAR_VCV’, ’NUM_EPOCHS_VCV’)
            rec_values = unpack(’>8fh’, self.msg_bytes[0:record_length])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 12:
            rec_field_names = (’POSITION_RMS_SIG’, ’SIG_EAST’, ’SIG_NORT’, ’COVAR_EN’, ’SIG_UP’,
                               ’SEMI_MAJOR’, ’SEMI_MINOR’, ’ORIENTATION’, ’UNIT_VAR_SIG’,
                               ’NUM_EPOCHS_SIG’)
            rec_values = unpack(’>9fh’, self.msg_bytes[0:record_length])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 15:
            rec_field_names = ’SERIAL_NUM’
            rec_values = unpack(’>l’, self.msg_bytes[0:record_length])
            self.rec_dict.update({rec_field_names: rec_values[0]})
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 16:
            rec_field_names = (’GPS_MS_OF_WEEK’, ’CT_GPS_WEEK’, ’UTC_OFFSET’, ’CT_FLAGS’)
            rec_values = unpack(’>l2hB’, self.msg_bytes[0:record_length])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 26:
            rec_field_names = (’UTC_MS_OF_WEEK’, ’UTC_GPS_WEEK’, ’UTC_SVS_NUM’, ’UTC_FLAG_1’, ’UTC_FLAG_2’,
                               ’UTC_INIT_NUM’)
            rec_values = unpack(’>lh4B’, self.msg_bytes[0:record_length])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        elif record_type == 34:
            NUM_OF_SVS = unpack(’>B’, self.msg_bytes[0])
            self.msg_bytes = self.msg_bytes[1:]
            rec_field_names = (’PRN’, ’SV_SYSTEM’, ’SV_FLAG1’, ’SV_FLAG2’, ’ELEVATION’, ’AZIMUTH’,
                               ’SNR_L1’, ’SNR_L2’, ’SNR_L5’)
            for field in xrange(len(rec_field_names)):
                self.rec_dict[rec_field_names[field]] = []
            for sat in xrange(NUM_OF_SVS[0]):
                rec_values = unpack(’>5Bh3B’, self.msg_bytes[0:10])
                self.msg_bytes = self.msg_bytes[10:]
                for num in xrange(len(rec_field_names)):
                    self.rec_dict[rec_field_names[num]].append(rec_values[num])
        elif record_type == 37:
            rec_field_names = (’BATT_CAPACITY’, ’REMAINING_MEM’)
            rec_values = unpack(’>hd’, self.msg_bytes[0:record_length])
            self.rec_dict.update(dict(zip(rec_field_names, rec_values)))
            self.msg_bytes = self.msg_bytes[record_length:]
        else:
            """Unknown record type? Skip it!"""
            #print record_type
            self.msg_bytes = self.msg_bytes[record_length:]
            
            
