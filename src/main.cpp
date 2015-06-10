#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/freq.h"
#include "../includes/myTypes.h"
#include "../includes/host.h"
#include "../includes/DVFS_Alg.h"
#include "../includes/myLib.h"
#include "../includes/examples.h"
#include <iostream>
#include <map>
using namespace std;

uint freqLvlNum = 0;
vector<double> freqs;
vector<vector<double> > fs;

vector<Host_IP> host_ip_list;
vector<Host> host_list;
Host host;
uint local_alg_id = ALG_VDS;
struct timeval sys_start_time;
struct timeval sys_curr_time;
bool is_show_message = false;

char ip_addrs[MAX_HOST_NUMBER][IP_ADDR_LEN] = {
    "127.0.0.1",
    "114.212.84.3",
    "114.212.85.151",
    "114.212.86.216"
};


int host_numbers = 1;
int g_finished_host_num;
pthread_mutex_t mutex;
vector<double> g_energy(host_numbers);
vector<uint> g_cpuid(host_numbers);
map<string, int> cmd_map;


void initial_cmd_map() {
    cmd_map["exit"] = cmd_exit;
    cmd_map["host"] = cmd_host;
    cmd_map["controller"] = cmd_controller;
    cmd_map["governor"] = cmd_governor;
    cmd_map["sysinit"] = cmd_sys_init;
    cmd_map["test"] = cmd_test;
    cmd_map["task"] = cmd_generate_tasks;
    cmd_map["simulation"] = cmd_sim_exp;
}
void test() {
    vector<Task> v_task;
    Task t1, t2, t3;
    t1.taskID = 3;
    t2.taskID = 2;
    t3.taskID = 1;
    t1.remainTime = 3;
    t2.remainTime = 2;
    t3.remainTime = 1;

    v_task.push_back(t1);
    v_task.push_back(t2);
    v_task.push_back(t3);

    Task *t = &v_task[0];
    cout << t->taskID << endl;
    sort(v_task.begin(), v_task.end());
    cout << t->taskID << endl;
}

int main(int argc, char const *argv[]) {
/*    initial_cmd_map();
    map<string,int>::iterator it;
    bool isRun = true;
 //   test();
    string cmd;
//    example_host_50();
    while(isRun) {
        cout << ">>>";
        cin >> cmd;
        it = cmd_map.find(cmd);
        if(it == cmd_map.end()) {
            cout << "Invalid command\n";
            continue;
        }
        switch (it->second) {
            case cmd_exit: {
                isRun = false;
                break;
            }
            case cmd_host: {
                thread_host();
                break;
            }
            case cmd_controller: {
                controller_main();
                break;
            }
            case cmd_governor: {
                cout << "===>Input governor: ";
                string governor;
                cin >> governor;
                change_governor(governor);
                break;
            }
            case cmd_sys_init: {
                send_msg_to_all(host_ip_list, REQ_HOST_INIT, NULL, 0);
                break;
            }
            case cmd_test: {
                test_module();
                break;
            }
            case cmd_generate_tasks: {
                uint task_num;
                double tasks_per_minute, min_task_len, max_task_len;
                cout << "   >>>Input numbers of task: ";
                cin >> task_num;
                cout << "   >>>Input tasks/minute: ";
                cin >> tasks_per_minute;
                cout << "   >>>Input min task length: ";
                cin >> min_task_len;
                cout << "   >>>Input max task length: ";
                cin >> max_task_len;
                generateTasks(task_num, tasks_per_minute, min_task_len, max_task_len);
                break;
            }
            case cmd_sim_exp: {
                sim_exp();
                break;
            }
            default:
                break;
        }
    }
    return 0;
//    example_task_control();*/
    run_one_task("bin/lu.A", 1900000, 0, 100);
}


