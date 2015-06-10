#ifndef _H_EXAMPLE_H_
#define _H_EXAMPLE_H_

#include "freq.h"
#include "myTypes.h"
#include "host.h"
#include "local_DVFS_Alg.h"
#include "allocation_Alg.h"
#include "hostMonitor.h"
#include "centralController.h"
#include "myLib.h"

extern uint freqLvlNum;
extern vector<double> freqs;
extern vector<vector<double> > fs;

extern vector<Host_IP> host_ip_list;
extern vector<Host> host_list;
extern Host host;

extern char ip_addrs[MAX_HOST_NUMBER][IP_ADDR_LEN];
extern bool is_show_message;

const int test_exit = 0;
const int test_host = 1;
const int test_my_alg = 2;
const int test_set_freq = 3;

void test_module();
void example_change_governor_freqs();
void example_host();
void example_controller();
void example_test1(int &a);
void example_get_cpuinfo();
void example_host_50();
void example_task_control();

void sim_exp();

#endif