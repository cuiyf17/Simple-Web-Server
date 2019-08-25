#ifndef PROCESSINFO_H
#define PROCESSINFO_H

#include "commonheader.h"

#define UID_LINE 7
#define VMRSS_LINE 17
#define VMSIZE_LINE 12

#define PID_ITEM 1
#define COMMAND_ITEM 2
#define STATE_ITEM 3
#define PPID_ITEM 4
#define PGID_ITEM 5
#define PROCESS_ITEM 14
#define TIME1_ITEM 14
#define CORE_ITEM 15
#define TIME2_ITEM 15
#define PR_ITEM 18
#define NI_ITEM 19
#define VIRT_ITEM 23
#define RES_ITEM 24

struct InfoProcess //保存进程信息的结构体
{
    unsigned int PID;   //PID，stat第1项
    char COMMAND[64];   //项目名称，stat第2项
    char STATE[2];      //进程当前的状态,stat第3项
    char USERNAME[64];  //进程所属用户名
    unsigned int PPID;  //PPID，stat第4项
    unsigned int PGID;  //PGID，stat第5项
    unsigned long TIME; //该进程的使用时间，stat第14+15项
    unsigned int PR;    //动态优先级，stat第18项
    unsigned int NI;    //静态优先级，stat第19项
    unsigned int VIRT;  //虚拟内存地址大小，stat第23项
    unsigned int RES;   //物理内存大小，stat第24项
    double CPUOCP;      //CPU占用率
};

typedef struct
{
    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
} Total_Cpu_Occupy_t;
typedef struct
{
    unsigned long utime;  //user time
    unsigned long stime;  //kernel time
    unsigned long cutime; //all user time
    unsigned long cstime; //all dead time
} Proc_Cpu_Occupy_t;

std::vector<InfoProcess> ReadProcesses();

//获取第N项开始的指针
const char *GetItems(const char *buffer, unsigned int item);

//获取总的CPU时间
unsigned long GetCPUTotalOccupy();

//获取进程的CPU时间
unsigned long GetCPUProcOccupy(unsigned int pid);

//获取CPU占用率
float GetProcCPU(unsigned int pid);

//获取进程UID
unsigned int GetUID(unsigned int pid);

//获取进程信息
std::vector<InfoProcess> GetProcessesInfo();

#endif