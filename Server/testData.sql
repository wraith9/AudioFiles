INSERT INTO Users (username, password, pw_salt) VALUES ('makarth', 'c5ee8e592dcf9c67982805134803a71c18064b9eee038c50249e8db54e8141ae67bd990508c3138d2b68a527f566d154d7f3f8893cae9e20eed7247fefafa826', 'abcde');
INSERT INTO Users (username, password, pw_salt) VALUES ('wally', 'd90424e23d3b2afb71274f90bc55a0d4d79270f67b29b62a4bf6d175fdcd7c7c619a961cc6dedfadd18a2cffc3e8ffb8f249108bbde37abf86bca340553f993e', 'hijkl');

INSERT INTO Friends (userID, friendID)
SELECT us.userID, fr.userID FROM Users us
JOIN Users fr ON fr.username = 'makarth'
WHERE us.username = 'wally';

INSERT INTO Friends (userID, friendID)
SELECT us.userID, fr.userID FROM Users us
JOIN Users fr ON fr.username = 'wally'
WHERE us.username = 'makarth';
