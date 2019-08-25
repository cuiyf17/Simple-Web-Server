#include "processinfo.h"

std::vector<InfoProcess> ReadProcesses()
{
    char basePath[] = "/proc";
    DIR *dir;
    struct dirent *ptr;
    if ((dir = opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        exit(1);
    }
    std::vector<InfoProcess> result;
    while ((ptr = readdir(dir)) != NULL)
    {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) ///current dir OR parrent dir
            continue;
        else if (ptr->d_name[0] >= '0' && ptr->d_name[0] <= '9') ///dir
        {
            InfoProcess temp;
            sscanf(ptr->d_name, "%u", &temp.PID);
            result.push_back(temp);
        }
    }
    closedir(dir);
    return result;
}

//获取第N项开始的指针
const char *GetItems(const char *buffer, unsigned int item)
{
    const char *p = buffer;
    int len = strlen(buffer);
    int count = 0;
    for (int i = 0; i < len; i++)
    {
        if (' ' == *p)
        {
            count++;
            if (count == item - 1)
            {
                p++;
                break;
            }
        }
        p++;
    }
    return p;
}
//获取总的CPU时间
unsigned long GetCPUTotalOccupy()
{
    FILE *fd;
    char buff[1024] = {0};
    Total_Cpu_Occupy_t t;
    fd = fopen("/proc/stat", "r");
    if (nullptr == fd)
    {
        return 0;
    }

    fgets(buff, sizeof(buff), fd);
    char name[64] = {0};
    sscanf(buff, "%s %ld %ld %ld %ld", name, &t.user, &t.nice, &t.system, &t.idle);
    fclose(fd);
    return (t.user + t.nice + t.system + t.idle);
}
//获取进程的CPU时间
unsigned long GetCPUProcOccupy(unsigned int pid)
{
    char file_name[64];
    Proc_Cpu_Occupy_t t;
    FILE *fd;
    char line_buff[1024];
    sprintf(file_name, "/proc/%d/stat", pid);
    fd = fopen(file_name, "r");
    if (nullptr == fd)
    {
        return 0;
    }
    fgets(line_buff, sizeof(line_buff), fd);
    const char *q = GetItems(line_buff, PROCESS_ITEM);
    sscanf(q, "%ld %ld %ld %ld", &t.utime, &t.stime, &t.cutime, &t.cstime);
    fclose(fd);
    return (t.utime + t.stime + t.cutime + t.cstime);
}
//获取CPU占用率
float GetProcCPU(unsigned int pid)
{
    unsigned long totalcputime1, totalcputime2;
    unsigned long procputime1, procputime2;
    totalcputime1 = GetCPUTotalOccupy();
    procputime1 = GetCPUProcOccupy(pid);
    usleep(200000);
    totalcputime2 = GetCPUTotalOccupy();
    procputime2 = GetCPUProcOccupy(pid);
    double pcpu = 0.0;
    if (0 != totalcputime2 - totalcputime1)
    {
        pcpu = 100.0 * (procputime2 - procputime1) / (totalcputime2 - totalcputime1);
    }
    return pcpu;
}

std::vector<InfoProcess> GetProcessesInfo()
{
    char str_PID[] = "PID";
    char str_USERNAME[] = "USER";
    char str_COMMAND[] = "COMMAND";
    char str_STATE[] = "STATE";
    char str_PPID[] = "PPID";
    char str_PGID[] = "PGID";
    char str_TIME[] = "TIME+(s)";
    char str_PR[] = "PR";
    char str_NI[] = "NI";
    char str_VIRT[] = "VIRT(KB)";
    char str_RES[] = "RES(KB)";
    char str_CPUOCP[] = "%CPU";
    std::vector<InfoProcess> ret = ReadProcesses();
    passwd *pw;
    for (std::vector<InfoProcess>::iterator i = ret.begin(); i != ret.end(); i++)
    {
        char file_name[64] = {0};
        FILE *fd;
        char line_buff[1024] = {0};
        sprintf(file_name, "/proc/%d/stat", i->PID);
        fd = fopen(file_name, "r");
        fgets(line_buff, sizeof(line_buff), fd);
        const char *p = line_buff;
        int len = strlen(line_buff);
        int count = 0;
        for (int j = 0; j < len; j++)
        {
            if (' ' == *p)
            {
                count++;
                if (count == COMMAND_ITEM - 1)
                {
                    p++;
                    sscanf(p, "%s", i->COMMAND);
                }
                else if (count == STATE_ITEM - 1)
                {
                    p++;
                    sscanf(p, "%s", i->STATE);
                }
                else if (count == PPID_ITEM - 1)
                {
                    p++;
                    sscanf(p, "%u", &i->PPID);
                }
                else if (count == PGID_ITEM - 1)
                {
                    p++;
                    sscanf(p, "%u", &i->PGID);
                }
                else if (count == TIME1_ITEM - 1)
                {
                    p++;
                    sscanf(p, "%lu", &i->TIME);
                }
                else if (count == TIME2_ITEM - 1)
                {
                    p++;
                    unsigned long int temp_int;
                    sscanf(p, "%lu", &temp_int);
                    i->TIME += temp_int;
                }
                else if (count == PR_ITEM - 1)
                {
                    p++;
                    sscanf(p, "%u", &i->PR);
                }
                else if (count == NI_ITEM - 1)
                {
                    p++;
                    sscanf(p, "%u", &i->NI);
                }

                else if (count == RES_ITEM - 1)
                {
                    p++;
                    sscanf(p, "%u", &i->RES);
                    i->RES = i->RES * getpagesize() / 1024;
                }
            }
            p++;
        }
        fclose(fd);
        i->CPUOCP = GetProcCPU(i->PID);
        unsigned int uid;
        sprintf(file_name, "/proc/%d/status", i->PID);
        fd = fopen(file_name, "r");
        char name[64];
        for (int j = 0; j < VMSIZE_LINE; j++)
        {
            fgets(line_buff, sizeof(line_buff), fd);
            if (j == UID_LINE - 1)
            {
                sscanf(line_buff, "%s%d", name, &uid);
            }
            else if (j == VMSIZE_LINE - 1)
            {
                sscanf(line_buff, "%s%d", name, &i->VIRT);
            }
        }
        fclose(fd);
        pw = getpwuid(uid);
        strcpy(i->USERNAME, pw->pw_name);
    }
    //FILE *fp = fopen("ProcessInfo.html", "w");
    int processinfo_fd = open("ProcessInfo.html", O_WRONLY);
    char buff[4096];
    memset(buff, '\0', sizeof(buff));
    sprintf(buff, "<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>\r\n", str_PID, str_USERNAME, str_STATE, str_PPID, str_PGID, str_PR, str_NI, str_VIRT, str_RES, str_CPUOCP, str_TIME, str_COMMAND);
    char head[4096];
    memset(head, '\0', sizeof(head));
    sprintf(head, "<html>\r\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><head><title>进程信息</title></head><body style=\"background-color:ivory\">\r\n<h1 style=\"background-color:rgba(137, 43, 226, 0.479); text-align:center\">进程信息</h1><hr /><table border=\"1\" style=\"text-align:center\">");
    char tail[] = "</body>\r\n</html>";
    char br[] = "<br />\r\n";
    char pre[] = "</pre>\r\n</tt>\r\n";
    char table[] = "</table>";
    strcat(head, buff);

    for (std::vector<InfoProcess>::iterator i = ret.begin(); i != ret.end(); i++)
    {
        memset(buff, '\0', sizeof(buff));
        sprintf(buff, "<tr><td>%u</td><td>%s</td><td>%s</td><td>%u</td><td>%u</td><td>%u</td><td>%u</td><td>%u</td><td>%u</td><td>%lf</td><td>%lu</td><td>%s</td></tr>\r\n", i->PID, i->USERNAME, i->STATE, i->PPID, i->PGID, i->PR, i->NI, i->VIRT, i->RES, i->CPUOCP, i->TIME, i->COMMAND);
        strcat(head, buff);
    }
    strcat(head, table);
    strcat(head, tail);

    write(processinfo_fd, head, sizeof(head));
    close(processinfo_fd);

    return ret;
}