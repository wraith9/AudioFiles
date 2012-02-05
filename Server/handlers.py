'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

import asyncore

BUFSIZE = 1400

class ChatHandler(asyncore.dispatcher):
    '''
    Handler for the ChatServer module -- here's where all the fun is
    '''
    
    def __init__(self, sock):
        asyncore.dispatcher.__init__(self, sock=sock)
        
    def handle_read(self):
        data = self.recv(BUFSIZE)
        if data:
            self.send(data)