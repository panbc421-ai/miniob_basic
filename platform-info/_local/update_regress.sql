create table r(id int, age int, name char(10), bd date);
insert into r values(1, 20, 'alice', '2020-01-01');
update r set age=99 where id=1;
select * from r where id=1;
exit;
