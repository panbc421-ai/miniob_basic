CREATE TABLE table_alias_1(id int, col1 int, feat1 float);
CREATE TABLE table_alias_2(id int, col2 int, feat2 float);
INSERT INTO table_alias_1 VALUES (7, 6, 56.2);
INSERT INTO table_alias_1 VALUES (27, 27, 17.4);
INSERT INTO table_alias_2 VALUES (43, 80, 15.2);
INSERT INTO table_alias_2 VALUES (54, 28, 48.3);
INSERT INTO table_alias_2 VALUES (73, 77, 32.4);
select t1.id as num, t1.col1 as age, t1.feat1, t2.* from table_alias_1 t1, table_alias_2 t2 where t1.id < t2.id;
exit;
