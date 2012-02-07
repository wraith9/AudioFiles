'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

from structlib import *
from common import *


loginFormatAck = padToSize(pktFormat.pack(e_invalidloginformat, 0), BUFSIZE)
loginInvalidAck = padToSize(pktFormat.pack(e_invalidlogin, 0), BUFSIZE)
unkAck = padToSize(pktFormat.pack(e_unknown, 0), BUFSIZE)

''' Handles login requests
    @return The packet data to send back to the client
'''
def handle_login(req):
    try:
        (username, pw), data = decode(clFormat, req)
    except error:
        return loginFormatAck
    del data # data is not used here
    # TODO: replace simple string checking with DB values
    username = username.replace(chr(0), '')
    pw = pw.replace(chr(0), '')
    if username == 'a' and pw == 'b':
        return padToSize(pktFormat.pack(r_login_good, 0), BUFSIZE)
    
    return loginInvalidAck

def handle_request(req):
    try:
        (pType, length), data = decode(pktFormat, req)
    except error:
        return unkAck
    
    if pType == r_login:
        return handle_login(data)
    else:
        return unkAck