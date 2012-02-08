'''
Created on Feb 4, 2012

@author: Tim Biggs
'''

from struct import Struct, error

pktFormat = Struct('!IBH')
clFormat = Struct('!20s20s')
flEntryFormat = Struct("!20sI")

def padToSize(string, maxSize):
    return string + ''.zfill(maxSize - len(string))

def decode(form, string):
    return (form.unpack(string[:form.size]), string[form.size:])