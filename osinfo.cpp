#include "osinfo.h"

void GetOsInfo()
{
    FILE *fp = fopen("/proc/version", "r");
    if (NULL == fp)
    {
        perror("(GetOsInfo)打开文件version失败!");
    }

    char szTest[1024];
    memset(szTest, '\0', sizeof(szTest));
    char head[4096];
    memset(head, '\0', sizeof(head));
    sprintf(head, "<html>\r\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><head><title>系统信息</title></head><body style=\"background-color:ivory\">\r\n<h1 style=\"background-color:rgba(137, 43, 226, 0.479); text-align:center\">系统信息</h1><hr /><pre>\r\n");
    char tail[] = "</body>\r\n</html>";

    while (!feof(fp))
    {
        memset(szTest, 0, sizeof(szTest));
        fgets(szTest, sizeof(szTest) - 1, fp); // leave out \n
        strcat(head, szTest);
    }
    strcat(head, tail);

    int osinfo_fd = open("OsInfo.html", O_WRONLY);
    write(osinfo_fd, head, sizeof(head));
    close(osinfo_fd);
    fclose(fp);
    //remove("OsInfo.html");
}