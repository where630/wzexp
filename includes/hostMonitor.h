#ifndef _H_HOST_MONITOR_H_
#define _H_HOST_MONITOR_H_

#include "myLib.h"
#include "allocation_Alg.h"
#include "local_DVFS_Alg.h"

extern Task allocated_task;
extern int allocated_cpuid;
extern vector<vector<double> > fs;
extern Host host;

void thread_host();
int deal_msg_host(char *buf);
void *run_real_task(void *pd);
int run_one_task(string task_name, int freq, int coreid, int utilization);
void set_all_same_freq();
void host_test();
void host_init();

#endif