create table exp_table(id int, col1 int, col2 int, col3 float, col4 float);
insert into exp_table VALUES (4, 2, 2, 4.3, 8.75);
insert into exp_table VALUES (8, 9, 7, 1.63, 3.67);
insert into exp_table VALUES (3, 4, 1, 5.37, 5.56);
select count(id) from exp_table where 1/7*3 < 9+col3*col3/9;
select min(col1)+avg(col2)*max(col3)/(max(col4)-4) from exp_table where id<>7/7;
exit;
