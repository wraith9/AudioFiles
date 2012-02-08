/* Author: Tim Biggs
   Purpose: Database build script
   
   Running: use the buildDB.sh script
   ./buildDB.sh [test]
   (Use './buildDB.sh test' to populate the DB with test data)
*/

CREATE TABLE Users(
   userID integer primary key,
   username varchar unique not null,
   password varchar not null,
   pw_salt varchar not null
);

CREATE TABLE Friends(
   id integer primary key,
   userID integer,
   friendID integer,
   FOREIGN KEY(userID) REFERENCES Users(userID),
   FOREIGN KEY(friendID) REFERENCES Friends(userID)
);
