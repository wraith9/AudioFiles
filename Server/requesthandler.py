'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

from structlib import *
from common import *
from struct import error
from itertools import izip_longest, starmap
import socket
import hashlib
import dao

daemonMap = dict()
dbdao = dao.dao()

def runUpdateDaemon(restartTimerFunc):
    updateDao = dao.dao()
    for userID in daemonMap.keys():
        if not userID in onlineFriends:
            onlineFriends[userID] = set([])
        
        friendListUpdate = set([])
        friendListOffline = set([])
        fullFriendList = updateDao.getFriends(userID)
        for (_, friendID) in fullFriendList:
            if friendID in onlineClients:
                friendListUpdate.add(friendID)
            else:
                friendListOffline.add(friendID)
        # Build lists (actually sets) of friends who went offline or online
        # Offline: Friend exists in old list but not in new one
        # Online: Friend exists in new list but not in old one
        offlinelist = onlineFriends[userID] & friendListOffline
        onlinelist = friendListUpdate - (onlineFriends[userID] & friendListUpdate)
        
        # Creating the actual data to send -- this uses some craziness with mapping functions
        # Basically, the first line does the offline list, and the second does the online one
        payload = ''.join(starmap(upFormat.pack, list(izip_longest(offlinelist, '', fillvalue=0))))
        payload = payload + ''.join(starmap(upFormat.pack, list(izip_longest(onlinelist, '', fillvalue=1))))
        
        onlineFriends[userID] = onlinelist
        sock = daemonMap[userID]
        sock.send(padToSize(pktFormat.pack(0, r_status_update, len(payload)) + payload, BUFSIZE))
    del updateDao
    restartTimerFunc()

class RequestHandler:
    
    def __init__(self, addr, sock):
        (hostaddr, _) = addr
        self.addr = hostaddr
        self.sock = sock
        
    def __del__(self):
        if not hasattr(self, 'userID'):
            return
        if self.userID in onlineClients:
            del onlineClients[self.userID]
        if self.userID in onlineFriends:
            del onlineFriends[self.userID]
        if self.userID in daemonMap:
            del daemonMap[self.userID]
    
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
    
    '''
    Called when a client daemon thread contacts the server with its own separate connection.
    This update daemon will use this connection to send periodic updates to the client.
    '''
    def addToDaemonList(self, userID):
        self.userID = userID
        if not userID in onlineClients:
            return errorPacket(e_not_logged_in)

        daemonMap[userID] = self.sock
        # Not actually an error packet -- just uses the same function to return a good status code
        return errorPacket(r_login_good)
    
    ''' Handles login requests
        @return The packet data to send back to the client
    '''
    def handle_login(self, req):
        try:
            (username, pw, dccpPort), _ = decode(clFormat, req)
        except error:
            return errorPacket(e_invalidloginformat)
        
        # note: this application not meant to be ultra secure, or even secure at all
        username = username.replace(chr(0), '')
        info = dbdao.getUser(username)
        if info == None:
            return errorPacket(e_invalidlogin)
        
        (userID, username, pw_hash, pw_salt) = info
        pw = pw.replace(chr(0), '')
        digest = hashlib.sha512(pw + pw_salt).hexdigest()

        # preventing duplicate logins
        if userID in onlineClients:
            return errorPacket(e_login_dup)
        if pw_hash == digest:
            self.userID = userID
            onlineClients[userID] = (self.addr, dccpPort)
            return self.init_friendlist_packet()
        
        return errorPacket(e_invalid_pw)
    
    def handle_addr_request(self, requesterID, data):
        def toIntVal(ipaddr):
            if ipaddr == '127.0.0.1':
                ipaddr = socket.gethostbyname(socket.gethostname())
            a = ipaddr.split('.')
            val = 0
            for elem in a:
                val = (val << 8) | int(elem)
            return val
        
        if not requesterID in onlineClients:
            return errorPacket(e_not_logged_in)
        
        try:
            (friendID, ), _ = decode(carFormat, data)
        except error:
            return errorPacket(e_unknown)
        
        # TODO: Filter for whether the two are friends or not
        if not friendID in onlineClients:
            return errorPacket(e_not_logged_in)
        
        (addr, port) = onlineClients[friendID]
        payload = carOutFormat.pack(toIntVal(addr), port)
        return padToSize(pktFormat.pack(0, r_request_addr, len(payload)) + payload, BUFSIZE)
    
    def handle_request(self, req):
        try:
            (userID, pType, _), data = decode(pktFormat, req)
        except error:
            return errorPacket(e_unknown)
        
        if pType == r_login:
            return self.handle_login(data)
        elif pType == r_request_addr:
            return self.handle_addr_request(userID, data)
        elif pType == r_daemon_init:
            return self.addToDaemonList(userID)
        else:
            return errorPacket(e_unknown)
