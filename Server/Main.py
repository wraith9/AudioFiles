'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

import asyncore
import server
import sys

if __name__ == '__main__':
    ipaddr = sys.argv[1]
    port = 9999
    if len(sys.argv) > 2:
        port = int(sys.argv[2])
    print "Listening on Address {}:{}".format(ipaddr, port)
    server.ChatServer(ipaddr, port)
    try:
        asyncore.loop()
    except KeyboardInterrupt:
        print "Stopping server..."
        asyncore.close_all()
        sys.exit()