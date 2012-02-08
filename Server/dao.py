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
        
    '''
    @return: None or the following tuple: (userID, username, password, salt)
    '''
    def getUser(self, username):
        param = (username, )
        self.cur.execute('SELECT * FROM Users WHERE username=?', param)
        return self.cur.fetchone()
    
    '''
    @return: A list of the following tuple: (friendname, friendID)
    '''
    def getFriends(self, userID):
        param = (userID, )
        self.cur.execute('SELECT us.username, fr.friendID FROM Friends fr ' +
                         'JOIN Users us ON us.userID = fr.friendID ' +
                         'WHERE fr.userID=?', param)
        return self.cur.fetchall()
        
    def __del__(self):
        self.conn.commit()
        self.cur.close()
        del self.conn
        del self.cur
        