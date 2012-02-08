'''
Created on Feb 7, 2012

@author: Tim Biggs
'''

import sqlite3

class dao:
    '''
    classdocs
    '''
    dbPath = 'ChatServerDB.db'

    def __init__(self):
        '''
        Constructor
        '''
        self.conn = sqlite3.connect(self.dbPath)
        self.cur = self.conn.cursor()
        
    def getUser(self, username):
        param = (username, )
        self.cur.execute('SELECT * FROM Users WHERE username=?', param)
        return self.cur.fetchone()
        
    def __del__(self):
        self.conn.commit()
        self.cur.close()
        del self.conn
        del self.cur
        