'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

from struct import Struct
from common import BUFSIZE

pktFormat = Struct('!IBH')
clFormat = Struct('!20s20sH')
flEntryFormat = Struct("!20sI")
carFormat = Struct("!I")
carOutFormat = Struct("!IH")
upFormat = Struct("!IB")

def padToSize(string, maxSize):
    return string + ''.zfill(maxSize - len(string))

def decode(form, string):
    return (form.unpack(string[:form.size]), string[form.size:])

def errorPacket(errorCode):
    return padToSize(pktFormat.pack(0, errorCode, 0), BUFSIZE)
