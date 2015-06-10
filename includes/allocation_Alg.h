#ifndef _H_ALLOCATION_ALG_H_
#define _H_ALLOCATION_ALG_H_

#include <vector>
#include "host.h"
#include "myLib.h"
#include "centralController.h"
#include "myTypes.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <queue>
#include <algorithm>

using namespace std;

typedef void (*Local_Alg)(Host *host, int maxSteps, vector<vector<double> > &state, vector<vector<double> > &freqSpace);


double min_energy_change(Host *host, Task *task, int &cpuid, uint runtimes, Local_Alg alg);

double sim_my_alg(vector<Host> &hostList, queue<Task> &taskQueue);
double sim_random_alloc(vector<Host> &hostList, queue<Task> &taskQueue);
double sim_first_fit(vector<Host> &hostList, queue<Task> &taskQueue);

void random_alloc(vector<Host> &host_list, vector<Host_IP> &ip_list, Task &task);
void first_fit(vector<Host> &host_list, vector<Host_IP> &ip_list, Task &task);
void my_alg(vector<Host> &host_list, vector<Host_IP> &ip_list, Task &task);

#endif