
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


using namespace std;

#define ATTR

void *thread_func(void *);
void sock_func();
void gai();


int main(int argc, char *argv[])
{
    bool gtk_en = false;
    bool fc_en = false;
    bool fifo_en = false;
    bool sock_en = false;
    bool gai_en = false;

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
        else if(strcmp("gai", argv[i])==0)
        {
            gai_en = true;
        }
    }

    if(sock_en)
    {
        sock_func();
    }

    if(gai_en)
    {
        gai();
    }

    if(fifo_en)
    {
        for(int i=1; i<sysconf(_SC_NPROCESSORS_ONLN); i++)
        {
            int status;
            unsigned long threadid;
            int *id = (int*)malloc(sizeof(int));
            *id = i;
#ifdef ATTR
            pthread_attr_t pthread_attr;
            struct sched_param param;
            (void)pthread_attr_init(&pthread_attr);
            (void)pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);
            pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);

            pthread_attr_setschedpolicy(&pthread_attr, SCHED_FIFO); // also, SCHED_RR
            param.sched_priority = 1;
            pthread_attr_setschedparam(&pthread_attr, &param);

            cpu_set_t cpuset;
            size_t cpusetsize = sizeof(cpu_set_t);
            CPU_ZERO(&cpuset);
            CPU_SET(i, &cpuset);
            status = pthread_attr_setaffinity_np(&pthread_attr, cpusetsize, &cpuset);

            status = pthread_create(&threadid, &pthread_attr, thread_func, (void*)id);
            printf("status %d \n", status);
#else
            status = pthread_create(&threadid, NULL, thread_func, (void*)id);
            printf("status %d \n", status);
#endif


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

    while(1)
    {
        int status;

        int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

        struct sockaddr_nl sa;
        memset(&sa, 0, sizeof(sa));
        sa.nl_family = AF_NETLINK;

        char buf[9000];

        memset(buf, 0, 9000);

        // assemble the message according to the netlink protocol
        struct nlmsghdr *nl;
        nl = (struct nlmsghdr*)buf;
        nl->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
        nl->nlmsg_type = RTM_GETADDR;
        nl->nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;

        struct ifaddrmsg *ifa;
        ifa = (struct ifaddrmsg*)NLMSG_DATA(nl);
        ifa->ifa_family = AF_UNSPEC;

        // prepare struct msghdr for sending.
        struct iovec iov = { nl, nl->nlmsg_len };
        struct msghdr msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };

        // send netlink message to kernel.
        int r = sendmsg(fd, &msg, 0);

        printf("sock send %d %d\n", ind++, r);

        close(fd);

        usleep(1000000);

    }

}



void gai()
{
    int ind = 0;
    while(1)
    {
        struct addrinfo hints, *ai, *aitop;
        char strport[] = "1234";
        int gaierr;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        if ((gaierr = getaddrinfo(NULL, strport, &hints, &aitop)) != 0)
            exit(-1);

        printf("gai %d \n", ind++);

        usleep(1000000);
    }
}

void *thread_func(void *args)
{
    int cpuid = *(int *)args;
    printf("forked to cpu %d \n", cpuid);

#ifndef ATTR
    int pid = syscall(SYS_gettid);
    cpu_set_t cpuset;
    size_t cpusetsize = sizeof(cpu_set_t);
    CPU_ZERO(&cpuset);
    CPU_SET(cpuid, &cpuset);
    sched_setaffinity(pid, cpusetsize, &cpuset);

    struct sched_param param;
    param.sched_priority = 1;
    int status = sched_setscheduler(pid, SCHED_FIFO, &param);
#endif

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
