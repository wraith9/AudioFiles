'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

import asyncore
import socket
import requesthandler
from common import BUFSIZE

class ChatHandler(asyncore.dispatcher):
    '''
    Handler for the ChatServer module -- 1 loop instance per client
    '''
    
    def __init__(self, sock):
        asyncore.dispatcher.__init__(self, sock=sock)
        
    def handle_read(self):
        self.send(requesthandler.handle_request(self.recv(BUFSIZE)))

class ChatServer(asyncore.dispatcher):
    '''
    Main server module -- should only be 1 instance in the loop
    '''

    def __init__(self, host, port):
        '''
        Constructor
        '''
        asyncore.dispatcher.__init__(self)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.set_reuse_addr()
        self.bind((host, port))
        self.listen(5)
    
    def handle_accept(self):
        pair = self.accept()
        if pair is None:
            pass
        else:
            sock,addr = pair
            print 'Incoming connection from %s' % repr(addr)
            ChatHandler(sock) # adds itself to the server loop automatically