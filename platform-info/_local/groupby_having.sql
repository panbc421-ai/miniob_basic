create table t_group_by (id int not null, score float not null, name char(1) null);
insert into t_group_by VALUES (1, 2.35, 'F');
insert into t_group_by VALUES (2, 1.44, '6');
insert into t_group_by VALUES (3, 2.04, '4');
insert into t_group_by VALUES (5, 2.02, '0');
insert into t_group_by VALUES (6, 8.42, 'A');
insert into t_group_by VALUES (7, 1.24, 'B');
insert into t_group_by VALUES (8, 1.66, 'C');
select id, sum(score) from t_group_by group by id having sum(score)>3.8609999999999998;
