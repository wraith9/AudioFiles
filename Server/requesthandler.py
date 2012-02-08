'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

from structlib import *
from common import *
import hashlib
import dao

dbdao = dao.dao()

# Constant packets -- will never change from user to user
loginValidAck = padToSize(pktFormat.pack(0, r_login_good, 0), BUFSIZE)
loginFormatAck = padToSize(pktFormat.pack(0, e_invalidloginformat, 0), BUFSIZE)
loginInvalidAck = padToSize(pktFormat.pack(0, e_invalidlogin, 0), BUFSIZE)
unkAck = padToSize(pktFormat.pack(0, e_unknown, 0), BUFSIZE)

''' Handles login requests
    @return The packet data to send back to the client
'''
def handle_login(req):
    try:
        (username, pw), _ = decode(clFormat, req)
    except error:
        return loginFormatAck
    
    # note: this application not meant to be ultra secure, or even secure at all
    username = username.replace(chr(0), '')
    info = dbdao.getUser(username)
    if info == None:
        return loginInvalidAck
    
    (_, username, pw_hash, pw_salt) = info
    pw = pw.replace(chr(0), '')
    digest = hashlib.sha512(pw + pw_salt).hexdigest()
    if pw_hash == digest:
        return loginValidAck
    
    return loginInvalidAck

def handle_request(req):
    try:
        (_, pType, _), data = decode(pktFormat, req)
    except error:
        return unkAck
    
    if pType == r_login:
        return handle_login(data)
    else:
        return unkAck