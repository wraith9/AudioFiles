'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

import asyncore
import server
import sys
import commands

if __name__ == '__main__':
    ipaddr = commands.getoutput('/bin/hostname -I').split("\n")[0].strip()
    port = 9999
    if len(sys.argv) > 1:
        port = int(sys.argv[1])
    print "Listening on Address {}:{}".format(ipaddr, port)
    server.ChatServer(ipaddr, port)
    try:
        asyncore.loop()
    except KeyboardInterrupt:
        print "Stopping server..."
        asyncore.close_all()
        sys.exit()