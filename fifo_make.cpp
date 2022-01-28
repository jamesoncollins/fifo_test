
#include <iostream>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <stdlib.h>
using namespace std;

void *thread_func(void *args);

int main(int argc, char *argv[])
{
    bool gtk_en = false;
    bool fc_en = false;
    bool fifo_en = false;

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
    }

    pthread_attr_t pthread_attr;
    struct sched_param param;
    (void)pthread_attr_init(&pthread_attr);
    (void)pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);

    if(fifo_en)
    {
        cout << "setting sched fifo" << endl;
        pthread_attr_setschedpolicy(&pthread_attr, SCHED_FIFO); // also, SCHED_RR
        param.sched_priority = 1;
        pthread_attr_setschedparam(&pthread_attr, &param);
    }

    int num_proc = sysconf(_SC_NPROCESSORS_ONLN);

    for(int i=min(2,num_proc-1); i<num_proc; i++)
    {
        unsigned long threadid;
        cpu_set_t cpuset;
        size_t cpusetsize = sizeof(cpu_set_t);
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);
        int status = pthread_attr_setaffinity_np(&pthread_attr, cpusetsize, &cpuset);
        status |= pthread_create(&threadid, &pthread_attr, thread_func, NULL);
        if(status)
            throw "pthread error";
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
    else
    {
        while(1)
        {
            usleep(100);
            sched_yield();
        }
    }

    return 0;

}

void *thread_func(void *args)
{
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
