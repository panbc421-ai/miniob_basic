CREATE TABLE null_table4(id int, num int null, name char(4) null);
INSERT INTO null_table4 VALUES (6, 6, 'N6');
INSERT INTO null_table4 VALUES (7, 7, 'N7');
INSERT INTO null_table4 VALUES (8, 8, 'N8');
UPDATE null_table4 SET num=null WHERE id=8;
SELECT * FROM null_table4;
UPDATE null_table4 SET name=null where num is null;
SELECT * FROM null_table4;
exit;
