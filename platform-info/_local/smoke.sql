create table t(id int, name char(10));
insert into t values(1,'a');
insert into t values(2,'b');
select * from t;
select length('abc') l1;
update t set name='zz' where id=2;
select * from t;
exit;
