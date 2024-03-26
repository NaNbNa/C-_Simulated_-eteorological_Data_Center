--删除T_ZHOBTMIND表1.5天前的数据
delete from T_ZHOBTMIND where ddatetime < sysdate - 1.5;
commit;

delete from T_ZHOBTMIND1 where ddatetime < sysdate -1.5;
commit;

exit;