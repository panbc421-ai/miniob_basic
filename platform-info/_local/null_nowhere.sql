CREATE TABLE t(id int, num int null);
INSERT INTO t VALUES (8, 8);
UPDATE t SET num=null;
SELECT * FROM t;
exit;
