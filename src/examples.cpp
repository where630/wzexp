#include "../includes/examples.h"
#include <map>
#include <iostream>
#include <string.h>

using namespace std;
map<string, int> test_cmd_map;

void test_module() {
    test_cmd_map["host"] = test_host;
    test_cmd_map["exit"] = test_exit;
    test_cmd_map["myalg"] = test_my_alg;
    test_cmd_map["freq"] = test_set_freq;
    bool isTestRun = true;
    string test_cmd;
    map<string,int>::iterator it;
    while(isTestRun) {
        cout << "--->>>";
        cin >> test_cmd;
        it = test_cmd_map.find(test_cmd);
        if(it == test_cmd_map.end()) {
            cout << "Invalid command\n";
            continue;
        }
        switch(it->second) {
            case test_host: {
                host_test();
                break;
            }
            case test_exit: {
                isTestRun = false;
                break;
            }
            case test_my_alg: {
                char buf[MAX_BUF_SIZE];
                vector<double> v;
                queue<Task> taskQueue;
                readTasks("tasks.txt", 1, taskQueue);
                Task *task = &taskQueue.front();

                send_msg_to_all(host_ip_list, REQ_MIN_ENERGY, task, sizeof(Task));
                break;
            }
            case test_set_freq: {
                set_all_same_freq();
                break;
            }
            default: {
                printf("default case\n");
                break;
            }
        }
    }
    cout << "===>exit test...\n";

}

void example_get_cpuinfo() {
    vector<double> cpu_utils;
    vector<double> cpu_freqs;
    get_cpu_utils(cpu_utils);
    get_cpu_freqs(cpu_freqs);
}

void example_change_governor_freqs() {
    char governor[] = "ondemand";
    setGovernor(0,2,6,governor);

}

void example_test1(int &a) {
    queue<Task> taskQueue;
    readTasks("allTask.txt", 6, taskQueue);
    Host_IP h1(1, ip_addrs[0]);
//    Host_IP h2(2, ip_addrs[1]);
    host_ip_list.push_back(h1);
//    host_ip_list.push_back(h2);

}

void example_controller() {
    char buf[MAX_BUF_SIZE];
    vector<double> v;
    queue<Task> taskQueue;
    Task *task = &taskQueue.front();
    /*double freqLvls[] = {2.395,2.261,2.128,1.995,1.862,1.729,1.596};
    Host h(1,2,6,2.33135,110.0,151.0);
    double freqs[] = {2.395,2.395};
    h.freqLvls.insert(h.freqLvls.end(), freqLvls, freqLvls+7);
    h.setCurrFreqs(freqs);*/

//    fs = freqSpaces(h.freqLvls, v, 2, 0);

    readTasks("../tasks.txt", 1, taskQueue);
/*    for(int i = 0; i < 6; i++) {
        h.allocateTo(taskQueue.front(), 0, freqs);
        taskQueue.pop();
    }
    for(int i = 0; i < 6; i++) {
        h.allocateTo(taskQueue.front(), 1, freqs);
        taskQueue.pop();
    }*/

    // vector<vector<double> > state;
    // alg_VDS(&h, 1000, state, fs);
    // Host h2 = h;
    // double energy = h.run(state);
    // alg_VDS(&h2, 0, state, fs);
    // double energy2 = h2.run(state);

    // printf("===>energy: %f %f\n", energy + h.getPStatic() * h.getCurrentTime(), energy2+ h.getPStatic() * h.getCurrentTime());
 //   printf("%u %f %f %f\n", task->taskID, task->startTime, task->deadline, task->userEstimateTime);

    send_msg_to_all(host_ip_list, REQ_MIN_ENERGY, &taskQueue.front(), sizeof(Task));
//    send_msg_to_all(host_ip_list, REQ_MIN_ENERGY, &taskQueue.front(), sizeof(Task));
}

void example_host() {
    uint core_num = sysconf(_SC_NPROCESSORS_ONLN);
    uint cups_at_same_freqs = cpus_with_same_freq();
    uint cpu_num = core_num / cups_at_same_freqs;
    host.setCpuNum(cpu_num);
    host.setCoreNum(core_num);
    getFreqLvls(0, host.freqLvls);
    host.setAlphaCPU(2.33135);
    host.setPStatic(110.0);
    host.setPMax(151.0);
    get_cpu_freqs(host.currCPUFreqs);
    get_cpu_utils(host.cpuUtils);

    vector<double> v;

    for(int i = 0; i < cpu_num; i++) {
        vector<Task> vt;
        host.taskList.push_back(vt);
    }
//    fs = freqSpaces(host.freqLvls, v, cpu_num, 0);

    queue<Task> taskQueue;
    readTasks("tasks.txt", 2, taskQueue);
    Task *task = &taskQueue.front();
    taskQueue.pop();
    for(int i = 0; i < 1; i++) {
        host.allocateTo(taskQueue.front(), 0, &(host.currCPUFreqs[0]));
        taskQueue.pop();
    }
    /*h.allocateTo(*task, 0, freqs);

    vector<vector<double> > state = alg_VDS(h,2,fs);
    double energy = h.run(state);
    printf("%f\n",energy);*/

    thread_host();
}


void example_one_host(int times) {
    /*double freqLvls[] = {2.395,2.261,2.128,1.995,1.862,1.729,1.596};
    Host h(1,4,6,2.33135,110.0,151.0);
    double freqs[] = {2.395,2.395, 2.395, 2.395};
    h.setUpUtil(1.0);
    h.freqLvls.insert(h.freqLvls.end(), freqLvls,freqLvls+7);
    h.setCurrFreqs(freqs);*/
    int host_num = 1;
    int task_num = 10;

    double freqLvls[] = {2.1, 2.0, 1.9, 1.8, 1.7, 1.6, 1.5, 1.4, 1.3, 1.2};
    Host h(1,12,1,0.140127,84.518,134.120);
    for(int i = 0; i < h.getCpuNum(); i++) {
        h.currCPUFreqs[i] = freqLvls[0];
    }
    h.setUpUtil(1.0);
    h.freqLvls.insert(h.freqLvls.end(), freqLvls,freqLvls+10);

    host_list.push_back(h);
    int i;
    for(i = 1; i < host_num; i++) {
        Host hostCopy = h;
        hostCopy.setHostID(i+1);
        host_list.push_back(hostCopy);
    }

    queue<Task> taskQueue;
    double energy;
    double total_time = readTasks("allTask.txt", 12, taskQueue);
    energy = total_time * host_list[0].getPStatic();

    for(int i = 0; i < h.getCpuNum(); i++) {
        for(int j = 0; j < 1; j++) {
            host_list[0].allocateTo(taskQueue.front(), i, NULL);
            taskQueue.pop();
        }
    }
cout << host_list[0].powerHost() << endl;
    bool is_obey_SLA = true;
    vector<vector<double> > state;
    clock_t start = clock();
    alg_VDS(&host_list[0], times, state, state);
    clock_t end = clock();
    printf("time is: %fs\n ", (end - start) / (double)CLOCKS_PER_SEC);
//    is_show_message = true;
    printf("...energy is: %f\n", energy + host_list[0].run(state, is_obey_SLA));
//    displayV2(state);
}

void example_host_50() {
    int num = 0;
    printf("----------iterations: %d----------\n", 0);
        example_one_host(0);
    for(int i = 0; i < 0; i++) {
        num *= 10;
        printf("----------iterations: %d----------\n", num);
        example_one_host(num);
    }
}

void example_task_control() {
    pthread_t pid;
    Real_Task rt;
    Task task;
    task.cpuUtil = 0.70;
    task.remainTime = 10;
    rt.task = &task;
    rt.cpuid = 0;

    int ret = 0;
    ret = pthread_create(&pid, NULL, run_real_task, &rt);

    if(ret != 0) {
        printf("Task Controll error\n");
        exit(1);
    }
    ret = pthread_join(pid, NULL);
    if(ret != 0) {
        printf("Thread join failed.\n");
    }
}


void exp_run(int host_num, Host h, queue<Task> qtask, int alg, double total_time) {
    int i;
    vector<Host> vhost;
    for(i = 0; i < host_num; i++) {
        Host hostCopy = h;
        hostCopy.setHostID(i+1);
        vhost.push_back(hostCopy);
    }


    double energy = h.getPStatic() * total_time;
    switch (alg) {
        case 0: {
            energy += sim_random_alloc(vhost, qtask);
            break;
        }
        case 1: {
            energy += sim_first_fit(vhost, qtask);
            break;
        }
        case 2: {
            energy += sim_my_alg(vhost, qtask);
            break;
        }
    }
    printf("===>total time is %f\n", total_time);
    printf("===>total energy is %f\n", energy);
}

void sim_exp() {
    int task_num, host_num;
    int alg;

    double total_time;

    cout << "   >>>Enter task numbers: ";
    cin >> task_num;
    cout << "   >>>Enter host numbers: ";
    cin >> host_num;

//    generateTasks(task_num, task_per_mintue);
    vector<Host> vhost;
    /*double freqLvls[] = {2.1, 2.0, 1.9, 1.8, 1.7, 1.6, 1.5, 1.4, 1.3, 1.2};
    Host h(1,12,1,0.140127,84.518,134.120);
    for(int i = 0; i < h.getCpuNum(); i++) {
        h.currCPUFreqs[i] = freqLvls[0];
    }
    h.setUpUtil(1.0);
    h.freqLvls.insert(h.freqLvls.end(), freqLvls,freqLvls+10);*/
//    h.setCurrFreqs(freqs);
    struct timeval tv_start;
    gettimeofday(&tv_start, NULL);
    double currentTime = tv_start.tv_sec + tv_start.tv_usec / 1000000.0;
    uint core_num = sysconf(_SC_NPROCESSORS_ONLN);
    uint cpu_num = core_num / 1;
    host.setCpuNum(cpu_num);
    host.setCoreNum(1);
    getFreqLvls(0, host.freqLvls);
    host.setAlphaCPU(0.140127);
    host.setPStatic(84.518);
    host.setPMax(134.120);
    host.setCurrentTime(currentTime);
    get_cpu_freqs(host.currCPUFreqs);
    get_cpu_utils(host.cpuUtils);

    host.taskList.erase(host.taskList.begin(), host.taskList.end());

    int i;
    for(i = 0; i < cpu_num; i++) {
        vector<Task> vt;
        host.taskList.push_back(vt);
    }
    queue<Task> qtask;
    total_time = readTasks("allTask.txt", task_num, qtask);

    for(int i = 0; i < 3; i++) {
        exp_run(host_num, host, qtask, i, total_time);
    }

}