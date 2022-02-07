
#include <iostream>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <gtk/gtk.h>
#include <stdlib.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <linux/rtnetlink.h>


using namespace std;

void *thread_func(void *);
void sock_func();


int main(int argc, char *argv[])
{
    bool gtk_en = false;
    bool fc_en = false;
    bool fifo_en = false;
    bool sock_en = false;

    for(int i=0; i<argc; i++)
    {
        if(strcmp("gtk", argv[i])==0)
        {
            gtk_en = true;
        }
        else if(strcmp("fc", argv[i])==0)
        {
            fc_en = true;
            gtk_en = true;
        }
        else if(strcmp("fifo", argv[i])==0)
        {
            fifo_en = true;
        }
        else if(strcmp("sock", argv[i])==0)
        {
            sock_en = true;
        }
    }

    if(sock_en)
    {
        sock_func();
    }

    if(fifo_en)
    {
        for(int i=1; i<sysconf(_SC_NPROCESSORS_ONLN); i++)
        {
            unsigned long threadid;
            int *id = (int*)malloc(sizeof(int));
            *id = i;
            pthread_create(&threadid, NULL, thread_func, (void*)id);
        }
        while(1)
            usleep(10000);
    }

    if(gtk_en)
    {
        cout << "init gtk" << endl;
        gtk_init(NULL, NULL);
        if(fc_en)
        {
            cout << "make file choose" << endl;
            GtkWidget *fc = gtk_file_chooser_button_new("title", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
        }
        gtk_main();
    }

    return 0;

}

void sock_func()
{
    int ind = 0;

    struct sockaddr_nl sa;
    static int sequence_number = 0;
    int status = 0;

    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    status = bind(fd, (struct sockaddr *) &sa, sizeof(sa));

    while(1)
    {
        printf("sock send %d\n", ind++);

        struct nlmsghdr nh;
        nh.nlmsg_pid = 0;
        nh.nlmsg_seq = ++sequence_number;
        nh.nlmsg_len = sizeof(struct nlmsghdr);
        //nh.nlmsg_flags |= NLM_F_ACK;
        struct iovec iov = { &nh, nh.nlmsg_len };
        struct msghdr msg;

        msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0};
        memset(&sa, 0, sizeof(sa));
        sa.nl_family = AF_NETLINK;
        status = sendmsg(fd, &msg, 0);

        usleep(1000000);

    }

    close(fd);
}

void *thread_func(void *args)
{
    int cpuid = *(int *)args;
    printf("forked to cpu %d \n", cpuid);

    int pid = syscall(SYS_gettid);
    cpu_set_t cpuset;
    size_t cpusetsize = sizeof(cpu_set_t);
    CPU_ZERO(&cpuset);
    CPU_SET(cpuid, &cpuset);
    sched_setaffinity(pid, cpusetsize, &cpuset);

    struct sched_param param;
    param.sched_priority = 1;
    int status = sched_setscheduler(pid, SCHED_FIFO, &param);

    int num_values = 200;
    int *test = (int *) calloc(num_values, 1);
    while(1)
    {
        sched_yield();
        for(int i=0; i<num_values; i++)
        {
            test[i] = i*2;
        }
    }
    return NULL;
}
