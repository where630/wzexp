#ifndef _H_FREQ_H_
#define _H_FREQ_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "myTypes.h"

using namespace std;

extern uint freqLvlNum;

void getFreqLvls(uint cpuid, vector<double> &cpu_freq_lvls);
int setGovernor(uint cpuid, uint cpuNum, uint sameCPUs, const char* governor);

int setFrequency(uint cpuid, uint cpuNum, uint sameCPUs, uint freq);

int change_governor(string governor);
void get_cpu_utils(vector<double> &cpu_utils);
void get_cpu_freqs(vector<double> &cpu_freqs);

uint cpus_with_same_freq();

#endif
