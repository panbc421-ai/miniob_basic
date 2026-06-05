CREATE TABLE null_table3(id int not null, num int null);
INSERT INTO null_table3 VALUES (1, null);
INSERT INTO null_table3 VALUES (2, null);
SELECT min(num) FROM null_table3;
