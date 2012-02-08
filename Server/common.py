'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

from structlib import pktFormat

BUFSIZE = 1400 + pktFormat.size
DATASIZE = 1400

r_login = 0x1
r_login_good = 0x2
r_login_failed = 0x3

e_invalidloginformat = 0x18
e_invalidlogin = 0x19
e_unknown = 0x20

onlineClients = dict()
