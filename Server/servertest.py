'''
Created on Feb 4, 2012

@author: Tim Biggs
'''
import unittest
import socket
import struct
import structlib

class Test(unittest.TestCase):
    maxsize = 1400 + struct.calcsize("!IBH")

    # TODO: Update tests as needed
    def testValidLogin(self):
        a = socket.create_connection(('localhost', 9999))
        a.send(structlib.padToSize(struct.pack("!IBH20s20s", 0, 1, 40, "makarth", "test"), self.maxsize))
        data = a.recv(self.maxsize)
        assert data == structlib.padToSize(struct.pack("!IBH20sI", 1, 0x2, 24, "wally", 2), self.maxsize)
        a.close()
        
    def testInvalidLogin(self):
        a = socket.create_connection(('localhost', 9999))
        a.send(structlib.padToSize(struct.pack("!IBH20s20s", 0, 1, 40, "makarth", "badpw"), self.maxsize))
        data = a.recv(self.maxsize)
        assert data == structlib.padToSize(struct.pack("!IBH", 0, 0x19, 0), self.maxsize)
        a.close()

if __name__ == "__main__":
    unittest.main()