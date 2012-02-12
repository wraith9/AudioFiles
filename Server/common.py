'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

from struct import calcsize

BUFSIZE = 1400 + calcsize("!IBH")
DATASIZE = 1400
UPDATE_INTERVAL = 4.0

r_login = 0x1
r_login_good = 0x2
r_login_failed = 0x3
r_status_update = 0x5
r_request_addr = 0x8

e_not_a_friend = 0x12
e_not_logged_in = 0x13
e_invalidloginformat = 0x14
e_invalidlogin = 0x15
e_invalid_pw = 0x16
e_unknown = 0x17

onlineClients = dict()
onlineFriends = dict()
