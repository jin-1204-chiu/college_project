USE demoDB;
GO

DROP TABLE Course;

CREATE TABLE Course
(cid int,
cname char(50),
pid int
);

insert into Course(cid, cname, pid) values (201, 'DB', 105), (203, 'OS', 105), (105, 'DS', NULL),
(101, 'CS_intro', NULL), (301, 'Security', 101);

select cname
from Course
where pid = (select cid from Course where cname = 'DS');
