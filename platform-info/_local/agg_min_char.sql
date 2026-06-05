CREATE TABLE aggregation_func(id int, num int, price float, addr char(4), birthday date);
INSERT INTO aggregation_func VALUES (60, 1, 11.0, 'GY', '2032-05-16');
INSERT INTO aggregation_func VALUES (40, 1, 16.0, 'OM', '1993-01-21');
INSERT INTO aggregation_func VALUES (10, 1, 1.0, 'T', '1998-10-20');
INSERT INTO aggregation_func VALUES (20, 1, 6.0, '7', '2028-08-06');
INSERT INTO aggregation_func VALUES (50, 1, 6.0, 'PW', '2020-01-29');
SELECT min(addr) FROM aggregation_func;
exit;
