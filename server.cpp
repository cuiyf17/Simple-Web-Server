#include "commonheader.h"
#include "threadpool.h"

#define BUFFSIZE 1024
#define MAXLINK 10 //未经过处理的连接请求队列可以容纳的最大数目
#define DEFAULT_PORT 8080

int main(int argc, char *argv[])
{
    char buff[BUFFSIZE];
    int socket_fd;
    int connect_fd;
    struct sockaddr_in server_address;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        perror("(服务器)Create socket error!");
        return -1;
    }
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = PF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(8080);

    assert(socket_fd >= 0);

    if (bind(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("(服务器)Bind error!");
        return -1;
    }
    if (listen(socket_fd, MAXLINK) == -1)
    {
        perror("(服务器)Listen error!");
    }

    CreateThreadPool(socket_fd);

    Run();
    //Wait();
    close(socket_fd);

    DelThreadPool();
    return 0;
}