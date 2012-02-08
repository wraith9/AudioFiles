dbname="ChatServerDB.db"

if [ -e $dbname ]; then
   rm $dbname
fi

sqlite3 $dbname ".read buildDB.sql"

if [ $# -gt 0 ]; then
   sqlite3 $dbname ".read testData.sql"
fi
