'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

import asyncore
import socket
import handlers

class ChatServer(asyncore.dispatcher):
    '''
    Main server module -- just kinda works like a server
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
            handlers.ChatHandler(sock)