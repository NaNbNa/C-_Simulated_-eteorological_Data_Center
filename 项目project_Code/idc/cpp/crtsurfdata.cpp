#include "_public.h"

using namespace idc;

struct st_stcode    //站点参数
{
    char provname[31];
    char obtid[11];
    char obtname[31];
    double lat;
    double lon;
    double height;
};
list<struct st_stcode> stlist;
struct st_surfdata   //观测数据
{
    char obtid[11];
    char ddatetime[15];
    int t;
    int p;
    int u;
    int wd;
    int wf;
    int r;      
    int vis;    
};
list<st_surfdata> datalist;

char strddatetime[15];


bool loadstcode(const string& infile);  //加载站点参数文件
void crtsurfdata(); //生成模拟观测数据
bool crtsurffile(const string &outpath,const string& datefmt);  //生成数据文件
void EXIT(int sig);

clogfile logfile;

cpactive pactive;

int main(int argc,char* argv[])
{
    if (argc!=5)
    {
        cout << "Using: ./crtsurfdata 站点参数文件 生成测试文件目录 运行日志文件路径 输出测试文件的格式" <<endl;
        cout << "Using:/project/tools/bin/procctl 60 /project/idc/bin/crtsurfdata /project/idc/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log csv,xml,json\n\n";

        cout << " 输出数据文件的格式:可以生成多个（格式），用 , 隔开\n";
        return -1;
    }
    
    closeioandsignal(true);
    signal(SIGINT,EXIT);signal(SIGTERM,EXIT);   //2,15

    pactive.addpinfo(10,"crtsurfdata");

    if (logfile.open(argv[3]) == false)
    {
        cout << "logfile() failed: " << argv[3] <<endl; return -1;
    }

    logfile.write("crtsurfdata 开始运行\n");


    //1
    if (loadstcode(argv[1]) == false) EXIT(-1);

    memset(strddatetime,0,sizeof(strddatetime));
    ltime(strddatetime,"yyyymmddhh24miss");
    strncpy(strddatetime + 12, "00",2);

    //2
    crtsurfdata();

    //3
    if (strstr(argv[4],"csv")!=0) crtsurffile(argv[2],"csv");
    if (strstr(argv[4],"json")!=0) crtsurffile(argv[2],"json");
    if (strstr(argv[4],"xml")!=0) crtsurffile(argv[2],"xml");

    logfile.write("crtsurfdata 运行结束\n");

    return 0;
}

void EXIT(int sig)
{
    logfile.write("procedure EXIT,sig=%d\n",sig);

    exit(0);
}

bool loadstcode(const string& infile)
{
    cifile ifile;

    if (ifile.open(infile) == false)
    {
        logfile.write("ifile open(%s) failed\n",infile.c_str()); return false;
    }

    string buffer;
    ifile.readline(buffer);

    ccmdstr cmdstr; //插分存储字符
    //重庆,57633,酉阳,28.47,108.48,419.3
    struct st_stcode stcode;

    while(ifile.readline(buffer) == true)   // 读取字符
    {
        //logfile.write("buffer=%s\n",buffer.c_str());
        cmdstr.splittocmd(buffer,",");
        memset(&stcode,0,sizeof(stcode));

        cmdstr.getvalue(0,stcode.provname,30);
        cmdstr.getvalue(1,stcode.obtid,10);
        cmdstr.getvalue(2,stcode.obtname,30);
        cmdstr.getvalue(3,stcode.lat);
        cmdstr.getvalue(4,stcode.lon);
        cmdstr.getvalue(5,stcode.height);

        stlist.push_back(std::move(stcode));
    }

    // for(auto aa: stlist)
    // {
    //     logfile.write("provname=%s,obtid=%s,obtname=%s,lat=%.2f,lon=%.2f,height=%.2f\n",\
    //                     aa.provname,aa.obtid,aa.obtname,aa.lat,aa.lon,aa.height);
    // }

    return true;
}

void crtsurfdata()  
{
    srand(time(0));

    st_surfdata stsurfdata;

    for(auto&aa: stlist)
    {
        memset(&stsurfdata,0,sizeof(stsurfdata));

        strcpy(stsurfdata.obtid,aa.obtid);      //obtid
        strcpy(stsurfdata.ddatetime,strddatetime);  //ddatetime
        stsurfdata.t = rand() %350;
        stsurfdata.p = rand() %265 + 10000;
        stsurfdata.u = rand() %101;
        stsurfdata.wd = rand() %360;
        stsurfdata.wf = rand() %150;
        stsurfdata.r = rand() %16;
        stsurfdata.vis = rand() %5001 +  100000;

        datalist.push_back(stsurfdata);
    }

    // for (auto& aa:datalist)
    // {
    //     logfile.write("%s,%s,%.1f,%.1f,%d,%d,%.1f,%.1f,%.1f\n",\
    //                      aa.obtid,aa.ddatetime,aa.t/10.0,aa.p/10.0,aa.u,aa.wd,aa.wf/10.0,aa.r/10.0,aa.vis/10.0);
    // }

    return;
}

bool crtsurffile(const string &outpath,const string& datafmt)
{
    string strfilename = outpath + "/" + "SURF_ZH_" + strddatetime + "_" + to_string(getpid()) + '.' + datafmt;

    cofile ofile;
    if (ofile.open(strfilename) == false)
    {
        logfile.write("ofile.open(%s)\n",strfilename.c_str());return false;
    }

    if (datafmt == "csv")   ofile.writeline("站点代码,据时间,气温,气压,相对湿度,风向,风速,降雨量,能见度\n");
    if (datafmt == "xml")   ofile.writeline("<data>\n");
    if (datafmt == "json")  ofile.writeline("{\"data\":[\n");

    for (auto&aa: datalist)
    {
        if (datafmt == "csv")
        {
            ofile.writeline("%s,%s,%.1f,%.1f,%d,%d,%.1f,%.1f,%.1f\n",\
                            aa.obtid,aa.ddatetime,aa.t/10.0,aa.p/10.0,aa.u,aa.wd,aa.wf/10.0,aa.r/10.0,aa.vis/10.0);
        }

        if (datafmt == "xml")
        {
            ofile.writeline("<obtid>%s</obtid>,<ddatetime>%s</ddatetime>,<t>%.1f</t>,<p>%.1f</p>," \
                            "<u>%d</u>,<wd>%d</wd>,<wf>%.1f</wf>,<r>%.1f</r>,<vis>%.1f</vis><endl/>\n",  \
                            aa.obtid,aa.ddatetime,aa.t/10.0,aa.p/10.0,aa.u,aa.wd,aa.wf/10.0,aa.r/10.0,aa.vis/10.0);
        }

        if (datafmt == "json")
        {
           ofile.writeline("{\"obtid\":\"%s\",\"ddatetime\":\"%s\",\"t\":\"%.1f\",\"p\":\"%.1f\"," \
                            "\"u\":\"%d\",\"wd\":\"%d\",\"wf\":\"%.1f\",\"r\":\"%.1f\",\"vis\":\"%.1f\"}",  \
                            aa.obtid,aa.ddatetime,aa.t/10.0,aa.p/10.0,aa.u,aa.wd,aa.wf/10.0,aa.r/10.0,aa.vis/10.0);
            static int ii=0;
            if (ii < datalist.size()-1)
            {
                ofile.writeline(",\n");ii++;
            }
            else
            {
                ofile.writeline("\n");
            }
        }
    }

    if (datafmt == "xml")   ofile.writeline("</data>\n");
    if (datafmt == "json")  ofile.writeline("]}\n");
    ofile.closeandrename();

    logfile.write("生成数据文件%s成功，数据时间%s，记录数%d\n",strfilename.c_str(),strddatetime,datalist.size());

    return true;
}
