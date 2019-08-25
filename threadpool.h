#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "commonheader.h"
#include "processinfo.h"
#include "osinfo.h"
#include "diskinfo.h"
#define BUFFSIZE 1024
#define MAXLINK 10           //未经过处理的连接请求队列可以容纳的最大数目
#define MAX_THREAD_NUMBER 16 //每个线程池最大的线程数

typedef void *(*FUNC)(void *arg);

class work
{
public:
    void *(*func)(void *arg);
    void *arg;

    work() : func(NULL), arg(NULL) {}
    work &operator=(const work &right)
    {
        this->arg = right.arg;
        this->func = right.func;
        return *this;
    }
};

class ThreadPool : public std::list<work>
{
public:
    pthread_mutex_t mutex;    //锁
    pthread_cond_t *cond;     //条件判断变量
    sem_t count;              //控制线程数目的信号量
    pthread_t *threads;       //线程池
    unsigned int maxnum;      //线程池能处理的最大线程数
    unsigned int number_wait; //线程池排队的任务数
    bool stop;                //停止标记
    int socket_fd;

    int *connect_fd_set;
    int set_num = 0;

public:
    ThreadPool(int socket_fd);
    ~ThreadPool();
};

void Wait();
int get_file_size(const char *fname);
char *get_file_name(char *buff);
void AddMission(FUNC func, void *arg);
void deal_get_http(int connect_fd, char *request_header);
bool is_get_http(char *buff);
void *Deal_Connect_fd(void *arg);
void working(int socket_fd);
void *doing(void *arg);
void Run();
void *Stop(void *arg);
ThreadPool *CreateThreadPool(int socket_fd);
void DelThreadPool();

#endif