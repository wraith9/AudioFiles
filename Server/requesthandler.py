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
loginFormatAck = padToSize(pktFormat.pack(0, e_invalidloginformat, 0), BUFSIZE)
loginInvalidAck = padToSize(pktFormat.pack(0, e_invalidlogin, 0), BUFSIZE)
unkAck = padToSize(pktFormat.pack(0, e_unknown, 0), BUFSIZE)

class RequestHandler:
    
    def __init__(self, addr):
        self.addr = addr
        
    def __del__(self):
        if not hasattr(self, 'userID'):
            return
        if self.userID in onlineClients:
            del onlineClients[self.userID]
    
    '''
    @param userID: The user id of the requester
    @return: A packet with a list of friends 
    '''
    def init_friendlist_packet(self):
        data = ''
        for friend in dbdao.getFriends(self.userID):
            if len(data) + flEntryFormat.size > DATASIZE:
                break
            (friendname, friendID) = friend
            data = data + flEntryFormat.pack(str(friendname), friendID)
        return padToSize(pktFormat.pack(self.userID, r_login_good, len(data)) + data, BUFSIZE)
    
    ''' Handles login requests
        @return The packet data to send back to the client
    '''
    def handle_login(self, req):
        try:
            (username, pw), _ = decode(clFormat, req)
        except error:
            return loginFormatAck
        
        # note: this application not meant to be ultra secure, or even secure at all
        username = username.replace(chr(0), '')
        info = dbdao.getUser(username)
        if info == None:
            return loginInvalidAck
        
        (userID, username, pw_hash, pw_salt) = info
        pw = pw.replace(chr(0), '')
        digest = hashlib.sha512(pw + pw_salt).hexdigest()
        # TODO: stop a client from logging in twice
        # if userID in onlineClients:
        #     return loginDuplicateAck
        if pw_hash == digest:
            self.userID = userID
            onlineClients[userID] = self.addr
            return self.init_friendlist_packet()
        
        return loginInvalidAck
    
    # TODO: Add more packet handlers
    def handle_request(self, req):
        try:
            (_, pType, _), data = decode(pktFormat, req)
        except error:
            return unkAck
        
        if pType == r_login:
            return self.handle_login(data)
        else:
            return unkAck