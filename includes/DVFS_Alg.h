#ifndef _H_DVFS_ALG_H_
#define _H_DVFS_ALG_H_

#include <vector>
#include "host.h"
#include "myLib.h"
#include "myTypes.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <queue>
#include <algorithm>

using namespace std;

typedef void (*Local_Alg)(Host *host, int maxSteps, vector<vector<double> > &state, vector<vector<double> > &freqSpace);


extern void alg_SA(Host *host, int maxSteps, vector<vector<double> > &state, vector<vector<double> > &freqSpace);
extern void alg_VDS(Host *host, int maxSteps, vector<vector<double> > &state, vector<vector<double> > &freqSpace);
extern double min_energy_change(Host *host, Task *task, uint &cpuid, uint runtimes, Local_Alg alg);

extern double sim_my_alg(vector<Host> &hostList, queue<Task> &taskQueue);
extern double sim_random_alloc(vector<Host> &hostList, queue<Task> &taskQueue);
extern double sim_first_fit(vector<Host> &hostList, queue<Task> &taskQueue);

extern void random_alloc(vector<Host> &host_list, vector<Host_IP> &ip_list, Task &task);
extern void first_fit(vector<Host> &host_list, vector<Host_IP> &ip_list, Task &task);
extern void my_alg(vector<Host> &host_list, vector<Host_IP> &ip_list, Task &task);

extern void neighbor(uint small,uint big, uint *result);
//extern double test();

#endif