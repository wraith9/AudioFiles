'''
Created on Feb 4, 2012

@author: Tim Biggs
'''
import unittest
import socket
import structlib

class Test(unittest.TestCase):
    maxsize = 1400

    # TODO: Update tests as needed
    def testValidLogin(self):
        a = socket.create_connection(('localhost', 9999))
        a.send(structlib.padToSize('\x01\x00(a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', self.maxsize))
        data = a.recv(self.maxsize)
        assert data == structlib.padToSize('\x02\x00\x00', self.maxsize)
        a.close()
        
    def testInvalidLogin(self):
        a = socket.create_connection(('localhost', 9999))
        a.send(structlib.padToSize('\x01\x00(ab', self.maxsize))
        data = a.recv(self.maxsize)
        assert data == structlib.padToSize('\x19\x00\x00', self.maxsize)

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()