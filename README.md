<div align="center">
    
# (C++)模拟气象数据中心
</div>

-------------------------------------------------------- 
### 项目文件和代码统计:    
![项目统计](https://github.com/NaNbNa/C-_Simulated_-eteorological_Data_Center/assets/144761706/2f332987-0976-400c-b5b8-e92432df2962)
#### 项目介绍：
涵盖项目的部分后台服务代码：
1. 项目框架：  
将C++操作再次封装，方便开发项目的模块。如字符串操作，日志操作等等  
2. 服务程序的守护程序和自启动脚本  
保证常驻服务程序和周期型服务程序的正常运行  
3.  数据入库和数据抽取  
将文件数据传入oracle；抽取Oracle数据到文件  
4. 数据同步  
实现多个Oracle数据库的数据同步备份，如备份一天前的数据  
5. 数据迁移  
避免Oracle的表过大，进行删除和修改操作，也可以使用插入操作替代删除和修改操作。  
6. 数据接口  
基于http协议，接受报文url，查询Oracle表中用户信息和权限，分析报文请求信心，返回相关Oracle数据库内的数据  
7. ftp和tcp文件传输  
文件传输，二进制方式。

#### 项目展示：
1. 后台服务程序  
   ![1711850278818](https://github.com/NaNbNa/C-_Simulated_-eteorological_Data_Center/assets/144761706/dd5eceb7-ee7c-4284-9900-164828f9280d)
2. 数据库设计  
![image](https://github.com/NaNbNa/C-_Simulated_-eteorological_Data_Center/assets/144761706/9202fa41-6230-494c-8a3e-eec49c8a7d97)
3. 项目结构  
![image](https://github.com/NaNbNa/C-_Simulated_-eteorological_Data_Center/assets/144761706/71e0975d-ef86-46ee-9d74-664059a256ad)


