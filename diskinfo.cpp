#include "diskinfo.h"

void TurnToTable(char buff[], char dest[])
{
    int j = 0;
    for (int i = 0; i < strlen(buff) + 2; i++)
    {
        if (!i)
        {
            strcat(dest, "<table border=\"1\"><tr><td>");
            j = j + 26;
            dest[j++] = buff[i];
        }
        else if (buff[i + 1] == 'M' && buff[i + 2] == 'o')
        {
            strcat(dest, "</td><td>");
            j = j + 9;
            strcat(dest, "Mounted on");
            j = j + 10;
            i = i + 10;
        }
        else if (buff[i] == ' ' && buff[i - 1] != ' ' && buff[i - 1] != '\n')
        {
            strcat(dest, "</td><td>");
            j = j + 9;
        }
        else if (buff[i - 1] == '\n')
        {
            strcat(dest, "<tr><td>");
            j = j + 8;
            dest[j++] = buff[i];
        }
        else if (buff[i] != ' ' && buff[i] != '\r' && buff[i] != '\n')
        {
            dest[j] = buff[i];
            ++j;
        }
        else if (buff[i] == '\n' && buff[i + 1] == '\0')
        {
            strcat(dest, "</td></tr></table>");
            break;
        }
        else if (buff[i] == '\n')
        {
            strcat(dest, "</td></tr>");
            j = j + 10;
        }
    }
}

void GetHardDiskInfo()
{
    const int filelength = 600;
    FILE *fd = popen("df -lh", "r");
    if (fd == NULL)
    {
        perror("(DiskInfo)创建新文件出错！");
        exit(1);
    }
    char buff[filelength];
    char dest[1024];
    memset(buff, '\0', sizeof(buff));
    memset(dest, '\0', sizeof(dest));
    fread(buff, filelength, 1, fd);
    TurnToTable(buff, dest);
    char head[4096];
    memset(head, '\0', sizeof(head));
    sprintf(head, "<html>\r\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><head><title>硬盘信息</title></head><body style=\"background-color:ivory\" >\r\n<h1 style=\"background-color:rgba(137, 43, 226, 0.479); text-align:center\">硬盘信息</h1><hr />");
    char tail[] = "</body>\r\n</html>";
    strcat(head, dest);
    strcat(head, tail);
    int diskinfo_fd = open("DiskInfo.html", O_WRONLY);
    write(diskinfo_fd, head, sizeof(head));
    close(diskinfo_fd);
    fclose(fd);
}