'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

import asyncore
import server
import sys

if __name__ == '__main__':
    server.ChatServer('localhost', 9999)
    try:
        asyncore.loop()
    except KeyboardInterrupt:
        print "Stopping server..."
        asyncore.close_all()
        sys.exit()