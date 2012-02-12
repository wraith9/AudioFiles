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
        assert data == structlib.padToSize(struct.pack("!IBH", 0, 0x16, 0), self.maxsize)
        a.close()
        
    def testFriendAddrRequest(self):
        a = socket.create_connection(('localhost', 9999))
        b = socket.create_connection(('localhost', 9999))
        a.send(structlib.padToSize(struct.pack("!IBH20s20s", 0, 1, 40, "makarth", "test"), self.maxsize))
        b.send(structlib.padToSize(struct.pack("!IBH20s20s", 0, 1, 40, "wally", "test"), self.maxsize))
        aData = a.recv(self.maxsize)
        assert aData == structlib.padToSize(struct.pack("!IBH20sI", 1, 0x2, 24, "wally", 2), self.maxsize)
        bData = b.recv(self.maxsize)
        assert bData == structlib.padToSize(struct.pack("!IBH20sI", 2, 0x2, 24, "makarth", 1), self.maxsize)
        a.send(structlib.padToSize(struct.pack("!IBHI", 1, 8, 4, 2), self.maxsize))
        pkt = struct.pack("!IBHI", 0, 8, 6, 2130706433)
        assert a.recv(self.maxsize)[:11] == pkt
        a.close()
        b.close()

if __name__ == "__main__":
    unittest.main()