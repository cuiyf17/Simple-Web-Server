#include "threadpool.h"
static ThreadPool *pool;
void Wait()
{
    for (int i = 0; i < pool->maxnum; i++)
    {
        pthread_join(pool->threads[i], NULL);
    }
}

int get_file_size(const char *fname)
{
    struct stat statbuf;
    if (!stat(fname, &statbuf))
    {
        return statbuf.st_size;
    }
    return -1;
}

char *get_file_name(char *buff)
{

    char *file_name = buff + 5;
    char *space = strchr(file_name, ' ');
    *space = '\0';
    return file_name;
}
void AddMission(FUNC func, void *arg)
{
    work worker;
    worker.func = func;
    worker.arg = arg;
    pthread_mutex_lock(&(pool->mutex));
    pool->push_back(worker);
    pool->number_wait++;
    pthread_mutex_unlock(&(pool->mutex));
    for (int i = 0; i < pool->maxnum; i++)
    {
        pthread_cond_signal(&(pool->cond[i]));
    }
}
void deal_get_http(int connect_fd, char *request_header)
{
    char *file_name = get_file_name(request_header);

    if (!strcmp(file_name, "ProcessInfo.html"))
    {
        GetProcessesInfo();
    }
    else if (!strcmp(file_name, "favicon.ico"))
    {
        return;
    }
    else if (!strcmp(file_name, "Stop"))
    {
        Stop(NULL);
        pthread_exit(NULL);
    }
    else if (!strcmp(file_name, "OsInfo.html"))
    {
        GetOsInfo();
    }
    else if (!strcmp(file_name, "DiskInfo.html"))
    {
        GetHardDiskInfo();
    }
    printf("访问%s\n", file_name);
    FILE *fd;
    fd = fopen(file_name, "r");
    char buff[4096];
    memset(buff, '\0', sizeof(buff));
    fread(buff, 1, 4096, fd);
    fclose(fd);

    char sen[4096];
    sprintf(sen, "HTTP/1.1 200 OK\r\n");
    sprintf(sen, "%sServer:CYFserver\r\n", sen);
    sprintf(sen, "%sContent-length:%lu\r\n", sen, strlen(buff));
    sprintf(sen, "%sContent-type:text/html\r\n\r\n", sen);
    sprintf(sen, "%s%s", sen, buff);
    send(connect_fd, sen, strlen(sen), 0);
}

bool is_get_http(char *buff)
{
    return !strncmp(buff, "GET", 3);
}

void *Deal_Connect_fd(void *arg)
{
    int connect_fd = (int)arg;
    char request[BUFFSIZE];
    memset(request, '\0', sizeof(request));
    recv(connect_fd, request, BUFFSIZE - 1, 0);
    if (is_get_http(request))
    {
        deal_get_http(connect_fd, request);
    }
    return NULL;
}

void working(int socket_fd)
{
    int connect_fd;
    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    connect_fd = accept(socket_fd, (struct sockaddr *)&client, &client_addrlength);
    pthread_mutex_lock(&pool->mutex);
    if (pool->set_num >= pool->maxnum)
    {
        pool->set_num = 0;
    }
    if (pool->connect_fd_set[pool->set_num])
    {
        close(pool->connect_fd_set[pool->set_num]);
    }
    pool->connect_fd_set[pool->set_num] = connect_fd;
    ++pool->set_num;
    pthread_mutex_unlock(&pool->mutex);
    if (connect_fd < 0)
    {
        perror("Accept error!");
    }
    else
    {
        int temp = connect_fd;
        AddMission(Deal_Connect_fd, reinterpret_cast<void *>(temp));
    }
}

void *doing(void *arg)
{
    int j = (int)arg;
    while (!pool->stop)
    {
        pthread_mutex_lock(&pool->mutex);
        while ((pool->empty()) && (!pool->stop))
        {
            pthread_cond_wait(&(pool->cond[j]), &(pool->mutex));
        }
        if (pool->stop)
        {
            pthread_mutex_unlock(&(pool->mutex));
            pthread_exit(NULL); //释放线程
        }
        assert(!pool->empty());
        --pool->number_wait; //等待任务减1,准备执行任务
        work worker = pool->front();
        pool->pop_front();
        pthread_mutex_unlock(&(pool->mutex));
        printf("(线程池)%d号子线程收到任务……\n", j);
        ((worker.func))(worker.arg); //执行回调函数
        printf("(线程池)%d号子线程完成任务！\n", j);
    }
    pthread_exit(NULL); // 释放线程
    return (void *)NULL;
}
ThreadPool *CreateThreadPool(int socket_fd)
{
    ThreadPool *tpool = new ThreadPool(socket_fd);
    pool = tpool;
    return tpool;
}
void DelThreadPool()
{
    if (pool != NULL)
    {
        delete pool;
    }
}
ThreadPool::ThreadPool(int socket_fd)
    : mutex(PTHREAD_MUTEX_INITIALIZER), maxnum(MAX_THREAD_NUMBER), number_wait(0), stop(0), socket_fd(socket_fd)
{
    sem_init(&count, 0, 0);
    connect_fd_set = new int[maxnum];
    threads = new pthread_t[this->maxnum];
    cond = new pthread_cond_t[this->maxnum];
    for (int i = 0; i < this->maxnum; i++)
    {
        cond[i] = PTHREAD_COND_INITIALIZER;
    }
}
ThreadPool::~ThreadPool()
{
    sem_destroy(&count);
    pthread_mutex_destroy(&mutex);
    for (int i = 0; i < maxnum; i++)
    {
        pthread_cond_destroy(&cond[i]);
        if (connect_fd_set[i])
        {
            close(connect_fd_set[i]);
        }
    }

    delete[] threads;
    delete[] cond;
    delete[] connect_fd_set;
}

void Run()
{
    for (int i = 0; i < pool->maxnum; i++)
    {
        int temp = i;
        pthread_create(&pool->threads[i], NULL, doing, reinterpret_cast<void *>(temp));
        printf("(线程池)%d号子线程创建成功……\n", i);
    }
    while (!pool->stop)
    {
        working(pool->socket_fd);
    }
}

void *Stop(void *arg)
{
    pool->stop = 1;
    for (int i = 0; i < pool->maxnum; i++)
    {
        pthread_cond_signal(&(pool->cond[i]));
    }
    printf("服务器即将关闭！\n");
    return NULL;
}
