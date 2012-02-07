'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

import asyncore
import server

if __name__ == '__main__':
    server.ChatServer('localhost', 9999)
    asyncore.loop()