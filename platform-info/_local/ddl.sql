create table create_table_select_t1(id int, name char(10));
insert into create_table_select_t1 values(1,'a');
insert into create_table_select_t1 values(2,'b');
create table create_table_select_t2 as select * from create_table_select_t1;
select * from create_table_select_t2;
create view create_view_v1 as select * from create_table_select_t1;
select * from create_view_v1;
exit;
