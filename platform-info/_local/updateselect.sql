CREATE TABLE Update_select_t1(id int, t_name char(10), col1 int, col2 int);
insert into Update_select_t1 VALUES (1, '0P585W', 5, 6);
insert into Update_select_t1 VALUES (1, 'RJ7', 2, 5);
insert into Update_select_t1 VALUES (2, '8MZ8B4', 2, 4);
insert into Update_select_t1 VALUES (5, 'OLD', 1, 2);
UPDATE Update_select_t1 SET t_name='9UBT4J5', col2=2 WHERE id=5;
SELECT * FROM Update_select_t1;
exit;
