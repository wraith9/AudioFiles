'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

import asynchat

BUFSIZE = 1400

class ChatHandler(asynchat.async_chat):
    '''
    Handler for the ChatServer module -- here's where all the fun is
    '''
    
    def __init__(self, sock):
        asynchat.async_chat.__init__(self, sock=sock)
        self.set_terminator('\r\n\r\n')
    
    def collect_incoming_data(self, data):
        if data:
            self.push(data.upper())
            
    def found_terminator(self):
        self.close()