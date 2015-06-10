#ifndef _H_MYLIB_H_
#define _H_MYLIB_H_

#include "math.h"
#include "host.h"
#include "myTypes.h"
#include "freq.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <queue>

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <algorithm>

using namespace std;

extern vector<Host> host_list;
extern vector<Host_IP> host_ip_list;
extern uint local_alg_id;
extern char ip_addrs[MAX_HOST_NUMBER][IP_ADDR_LEN];

const int runtime_vds = 100;
const int runtime_sa = 1000;

extern int g_finished_host_num;
extern pthread_mutex_t mutex;
extern vector<double> g_energy;
extern vector<uint> g_cpuid;

/*随机函数*/
double myRand();
double normalRand(const double min, const double sigma);

/*任务函数*/
double readTasks(const char *filepath, uint task_num, queue<Task> &taskQueue);
void generateTasks(const uint task_num, const double task_per_minute, double min_task_len, double max_task_len);

vector<vector<double> > freqSpaces(vector<double> &freqLvls, vector<double> freqs, uint n, uint depth);

int writeFile(const char* filepath, const char* mode, const char* content);

void displayV1(vector<double> &v1);
void displayV2(vector<vector<double> > &v2);


#endif