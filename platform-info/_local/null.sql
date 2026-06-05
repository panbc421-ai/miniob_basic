CREATE TABLE null_table(id int not null, num int null, price float not null, birthday date null);
insert into null_table VALUES (6, 5, 35.21, '2010-04-29');
insert into null_table VALUES (4, 4, 41.64, '2011-09-01');
insert into null_table VALUES (2, null, 20.0, '2010-01-11');
SELECT * FROM null_table where 9 is null;
SELECT * FROM null_table where 9 is not null;
SELECT * FROM null_table where num is null;
SELECT * FROM null_table where num is not null;
exit;
