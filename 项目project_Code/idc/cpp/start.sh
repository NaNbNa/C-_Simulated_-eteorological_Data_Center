# 守护模块
#/project/tools/bin/procctl 10 /project/tools/bin/checkproc /tmp/log/checkproc.log

#生成测试数据
/project/tools/bin/procctl 60 /project/idc/bin/crtsurfdata /project/idc/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log csv,xml,json

#定期删除历史文件
/project/tools/bin/procctl 300 /project/tools/bin/deletefiles /tmp/idc/surfdata "*.xml,*.json,*.csv" 0.02
#定期删除历史文件
# /project/tools/bin/procctl 300 /project/tools/bin/deletefiles /log/idc "*.log.20*" 0.25

#定期压缩历史文件
/project/tools/bin/procctl 300 /project/tools/bin/gzipfiles /log/idc "*.log.20*" 0.05


# --------------- 基于fcp协议的文件传输
# 下载后台服务程序/tmp/idc/surfdata的的备份日志,放在客户端的/idcdata/surfdata
/project/tools/bin/procctl 30 /project/tools/bin/ftpgetfiles /log/idc/ftpgetfiles_surfdata.log "<host>127.0.0.1:21</host><mode>1</mode><username>test1</username><password>1</password><remotepath>/tmp/idc/surfdata</remotepath><localpath>/idcdata/surfdata</localpath><matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname><ptype>1</ptype><remotepathbak>/tmp/idc/surfdatabak</remotepathbak><okfilename>/idcdata/ftplist/ftpgetfiles_test.xml</okfilename><checkmtime>true</checkmtime><timeout>30</timeout><pname>ftpgetfiles_test</pname>"
                                                                                

# 删除客户端/idcdata/surfdata的历史文件
 /project/tools/bin/procctl 300 /project/tools/bin/deletefiles /idcdata/surfdata "*" 0.04


# 将本地文件/tmp/idc/surfdata上传到/tmp/ftpputest
# 服务器要创建好/tmp/ftpputest目录(仅限于ptype=1,如果是其他ptype,也需要创建对应目录)
/project/tools/bin/procctl 30 /project/tools/bin/ftpputfiles /log/idc/ftpputfiles_surfdata.log "<host>127.0.0.1:21</host><mode>1</mode><username>test1</username><password>1</password><remotepath>/tmp/ftpputest</remotepath><localpath>/tmp/idc/surfdata</localpath><matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname><ptype>1</ptype><localpathbak>/tmp/idc/surfdatabak</localpathbak><okfilename>/idcdata/ftplist/ftpputfiles_test.xml</okfilename><checkmtime>true</checkmtime><timeout>30</timeout><pname>ftpputfiles_test</pname>"
                                                                                       

# 删除服务端/tmp/ftpputest的历史文件
 /project/tools/bin/procctl 300 /project/tools/bin/deletefiles /tmp/ftpputest "*" 0.04

# --------------- 基于tcp协议的文件传输
# 文件传输(get/put)的服务端程序,注意端口
 /project/tools/bin/procctl 10 /project/tools/bin/fileserver 5005 /log/idc/fileserver.log

# 从 /tmp/ftpputest 下载文件到 /tmp/tcpputest
/project/tools/bin/procctl 20  /project/tools/bin/tcpputfiles /log/idc/tcpputfiles.log  "<ip>127.0.0.1</ip><port>5005</port><clientpath>/tmp/ftpputest</clientpath><ptype>1</ptype><matchname>*.XML,*.JSON,*.CSV</matchname><andchild>true</andchild><timetvl>10</timetvl><srvpath>/tmp/tcpputest</srvpath><timeout>50</timeout><pname>tcpputfiles_surfdata</pname>"

# 从 /tmp/tcpputest 下载文件到 /tmp/tcpgetest 
/project/tools/bin/procctl 20  /project/tools/bin/tcpgetfiles /log/idc/tcpgetfiles.log  "<ip>127.0.0.1</ip><port>5005</port><clientpath>/tmp/tcpgetest</clientpath><ptype>1</ptype><matchname>*.XML,*.JSON,*.CSV</matchname><andchild>true</andchild><timetvl>10</timetvl><srvpath>/tmp/tcpputest</srvpath><timeout>50</timeout><pname>tcpgetfiles_surfdata</pname>"
                                                                             

# 清理/tmp/tcpgetest
 /project/tools/bin/procctl 300 /project/tools/bin/deletefiles /tmp/tcpgetest/ "*" 0.02

# 把/idcdata/surfdata目录中的气象观测数据文件入库到T_ZHOBTMIND表中
 /project/tools/bin/procctl 10 /project/idc/bin/obtmindtodb /idcdata/surfdata "idc/idcpwd@snorcl11g_131" "Simplified Chinese_China.AL32UTF8" /log/idc/obtmindtodb.log

# 执行/project/idc/sql/deletetable.sql脚本，删除T_ZHOBTMIND表两小时前的数据
 /project/tools/bin/procctl 120 /oracle/home/bin/sqlplus idc/idcpwd@snorcl11g_131 @/project/idc/sql/deletetable.sql

# 每隔一个小时从T_ZHOBTCODE表中抽取全部数据到/idcdata/dmindata
 /project/tools/bin/procctl 3600 /project/tools/bin/dminingoracle /log/idc/dminingorcle_ZHOBTCODE.log "<connstr>idc/idcpwd@snorcl11g_131</connstr><charset>Simplified Chinese_China.AL32UTF8</charset><selectsql>select obtid,cityname,provnname,lat,lon,height from T_ZHOBTCODE</selectsql><fieldstr>obtid,cityname,provnname,lat,lon,height</fieldstr><fieldlen>10,30,30,10,10,10</fieldlen><bfilename>ZHOBTCODE</bfilename><efilename>toidc</efilename><outpath>/idcdata/dmindata</outpath><timeout>30</timeout><pname>dminingoracle_ZHOBTCODE</pname>"

# 每30秒从T_ZHOBTMIND表中增量抽取数据到/idcdata/dmindata
/project/tools/bin/procctl 30 /project/tools/bin/dminingoracle /log/idc/dminingorcle_ZHOBTMIND.log "<connstr>idc/idcpwd@snorcl11g_131</connstr><charset>Simplified Chinese_China.AL32UTF8</charset><selectsql>select obtid,to_char(ddatetime,'yyyymmddhh24miss'),t,p,u,wd,wf,r,vis,keyid from T_ZHOBTMIND where keyid>:1 and obtid like '5%%'</selectsql><fieldstr>obtid,ddatetime,t,p,u,wd,wf,r,vis,keyid</fieldstr><fieldlen>5,19,8,8,8,8,8,8,8,15</fieldlen><bfilename>ZHOBTMIND</bfilename><efilename>togxpt</efilename><outpath>/idcdata/dmindata</outpath><starttime></starttime><incfield>keyid</incfield><incfilename>/idcdata/dmining/dminingoracle_ZHOBTMIND_togxpt.keyid</incfilename><timeout>30</timeout><pname>dminingoracle_ZHOBTMIND_togxpt</pname><maxcount>1000</maxcount><connstr1>scott/a@snorcl11g_131</connstr1>"

# 清理/idcdata/dimindata目录中的文件，防止空间崩溃
/project/tools/bin/procctl 300 /project/tools/bin/deletefiles /idcdata/dmindata "*" 0.02

# 把/idcdata/dmindata目录中的xml文件发送到/idcdata/xmltodb/vip，交给入库程序。
/project/tools/bin/procctl 20 /project/tools/bin/tcpputfiles /log/idc/tcpputfiles_togxpt.log "<ip>127.0.0.1</ip><port>5005</port><ptype>1</ptype><clientpath>/idcdata/dmindata</clientpath><srvpath>/idcdata/xmltodb/vip</srvpath><andchild>true</andchild><matchname>*.XML</matchname><timetvl>10</timetvl><timeout>50</timeout><pname>tcpputfiles_togxpt</pname>"

# 把/idcdata/xmltodb/vip目录中的xml文件入库到T_ZHOBTCODE1和T_ZHOBTMIND1。
/project/tools/bin/procctl 10 /project/tools/bin/xmltodb /log/idc/xmltodb_vip.log "<connstr>idc/idcpwd@snorcl11g_131</connstr><charset>Simplified Chinese_China.AL32UTF8</charset><inifilename>/project/idc/ini/xmltodb.xml</inifilename><xmlpath>/idcdata/xmltodb/vip</xmlpath><xmlpathbak>/idcdata/xmltodb/vipbak</xmlpathbak><xmlpatherr>/idcdata/xmltodb/viperr</xmlpatherr><timetvl>5</timetvl><timeout>50</timeout><pname>xmltodb_vip</pname>"
# 注意，观测数据源源不断的入库到T_ZHOBTMIND1中，为了防止表空间被撑满，在/project/idc/sql/deletetable.sql中要配置清理T_ZHOBTMIND1表中历史数据的脚本。

# 清理/idcdata/xmltodb/vipbak和/idcdata/xmltodb/viperr目录中文件。
/project/tools/bin/procctl 300 /project/tools/bin/deletefiles /idcdata/xmltodb/vipbak "*" 0.02
/project/tools/bin/procctl 300 /project/tools/bin/deletefiles /idcdata/xmltodb/viperr  "*" 0.02

# 清理T_ZHOBTMIND表中1天之前的数据。
/project/tools/bin/procctl 3600 /project/tools/bin/deletetable /log/idc/deletetable_ZHOBTMIND.log "<connstr>idc/idcpwd@snorcl11g_131</connstr><tname>T_ZHOBTMIND</tname><keycol>rowid</keycol><where>where ddatetime<sysdate-1</where><maxcount>100</maxcount><timeout>120</timeout><pname>deletetable_ZHOBTMIND</pname>"

# 把T_ZHOBTMIND1表中1天之前的数据迁移到T_ZHOBTMIND1_HIS表
/project/tools/bin/procctl 3600 /project/tools/bin/migratetable /log/idc/migratetable_ZHOBTMIND1.log "<connstr>idc/idcpwd@snorcl11g_131</connstr><tname>T_ZHOBTMIND1</tname><totname>T_ZHOBTMIND1_HIS</totname><keycol>rowid</keycol><where>where ddatetime<sysdate-1</where><maxcount>100</maxcount></starttime><timeout>120</timeout><pname>migratetable_ZHOBTMIND1</pname>"

# 清理T_ZHOBTMIND1_HIS表中1天之前的数据。
/project/tools/bin/procctl 3600 /project/tools/bin/deletetable /log/idc/deletetable_ZHOBTMIND1_HIS.log "<connstr>idc/idcpwd@snorcl11g_131</connstr><tname>T_ZHOBTMIND1_HIS</tname><keycol>rowid</keycol><where>where ddatetime<sysdate-1</where><maxcount>100</maxcount><timeout>120</timeout><pname>deletetable_ZHOBTMIND1_HIS</pname>"

