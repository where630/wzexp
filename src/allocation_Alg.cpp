#include "../includes/allocation_Alg.h"
#include "../includes/local_DVFS_Alg.h"

bool is_task_allocated;

double getMinTimeInHosts(vector<Host> hostList);

double sim_my_alg(vector<Host> &hostList, queue<Task> &taskQueue) {
    double energy = 0.0;
    bool isFinished = false,isAllocated, isObeySAL = false;
    Task *task;
    Host *host;
    double timeLen;
    double currentTime = 0.0;
    double min_energy_change, e1, e2;
//    double *pfreqs;
    uint i,j, slaCounter = 0;
    uint cpu_index;
    int host_index;
    Host copy1;
    vector<double> freqs, best_freqs;
    host = &hostList[0];
    Host h1, h2;
    vector<vector<double> > fs; // =  freqSpaces(host->freqLvls, freqs, host->getCpuNum(), 0);
    vector<vector<double> > state;
    freqs.erase(freqs.begin(), freqs.end());
    for(i = 0; i < host->getCpuNum(); i++) {
        freqs.push_back(host->freqLvls[0]);
    }
    cout << ">>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<\n";
    clock_t start = clock();
    bool is_obey_SLA = true;
    while(!isFinished) {
        is_obey_SLA = true;
        if(0 == taskQueue.size()) {
            timeLen = 1.0;
//            printf("randomAllocation:*****%d %d*****\n", (int)host->taskList[0].size(),(int)host->taskList[1].size());
            while(fabs(timeLen - 0.0) > 1e-10){
                timeLen = getMinTimeInHosts(hostList);
                currentTime += timeLen;
                for(i = 0; i < hostList.size(); i++) {
                    host = &hostList[i];
                    energy = energy + host->runOneT(timeLen, is_obey_SLA);
                    host->setCurrentTime(currentTime);
                }
            }
            isFinished = true;
        }
        else{
            task = &taskQueue.front();
            cpu_index = -1;
            host_index = -1;
            timeLen = getMinTimeInHosts(hostList);
            if(fabs(timeLen - 0.0)< 1e-10) {
                timeLen = 1990063019910621.0;
            }
            if(task->startTime < timeLen + currentTime){
                isAllocated = false;
                isObeySAL = false;
                timeLen = task->startTime - currentTime;
                min_energy_change = 1e100;
//                printf("randomAllocation:*****%f*****\n", timeLen);
                for(i = 0; i < hostList.size();i++) {
                    host = &hostList[i];
                    energy += host->runOneT(timeLen, is_obey_SLA);
                    host->setCurrentTime(task->startTime);
                    h2 = *host;
                    alg_VDS(&h2, 100, state, fs);
                    e2 = h2.run(state, is_obey_SLA);
                    for(j = 0; j < host->getCpuNum(); j++) {
                        h1 = *host;
//                        freqs = state[0];
                        if(h1.allocateTo(*task, j, &freqs[0])) {
                            alg_VDS(&h1, 100, state, fs);
                            e1 = h1.run(state, is_obey_SLA);
                            if((e1 - e2) < min_energy_change) {
                                min_energy_change = e1 - e2;
                                cpu_index = j;
                                host_index = i;
                                best_freqs = state[0];
                            }
                        }
                    }
                }
                if(host_index == -1) {
                    slaCounter++;
                }
                else{
                    host = &hostList[host_index];
                    host->allocateTo(*task, cpu_index, NULL);
                    host->changeFreqs(&best_freqs[0]);
                }
                taskQueue.pop();
                currentTime += timeLen;
            }
            else {
                currentTime += timeLen;
                for(i = 0; i < hostList.size();i++) {
                    host = &hostList[i];
                    energy += host->runOneT(timeLen, is_obey_SLA);
                    host->setCurrentTime(currentTime);
                }

            }
        }
    }
    clock_t end = clock();
    printf("===>my alg running time: %f s\n",(double)(end-start)/CLOCKS_PER_SEC);
    printf("===>SLAs : %d \n", slaCounter);
    return energy;
}

double sim_random_alloc(vector<Host> &hostList, queue<Task> &taskQueue) {
    if(hostList.size() == 0 || taskQueue.size() == 0) return 0;

    double energy = 0.0;
    bool isFinished = false,isAllocated, isObeySAL = false;
    Task *task;
    Host *host;
    uint cpuIndex;
    double timeLen;
    double currentTime = 0.0;
    uint i;
    uint selTimes,rand_Host_Index,slaCounter = 0;
    Host copy1;
    vector<double> freqs;
    host = &hostList[0];
    for(i = 0; i < host->getCpuNum(); i++) {
        freqs.push_back(host->freqLvls[0]);
    }
    bool is_obey_SLA = true;
    cout << ">>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<\n";
    clock_t start = clock();
    while(!isFinished) {
        is_obey_SLA = true;
        if(0 == taskQueue.size()) {
            timeLen = 1.0;
//            printf("randomAllocation:*****%d %d*****\n", (int)host->taskList[0].size(),(int)host->taskList[1].size());
            while(fabs(timeLen - 0.0) > 1e-10){
                timeLen = getMinTimeInHosts(hostList);
                currentTime += timeLen;
                for(i = 0; i < hostList.size(); i++) {
                    host = &hostList[i];
                    energy = energy + host->runOneT(timeLen, is_obey_SLA);
                    host->setCurrentTime(currentTime);
                }
            }
            isFinished = true;
        }
        else{
            task = &taskQueue.front();
            timeLen = getMinTimeInHosts(hostList);
            if(fabs(timeLen - 0.0)< 1e-10) {
                timeLen = 1990063019910621.0;
            }
            if(task->startTime < timeLen + currentTime){
                isAllocated = false;
                timeLen = task->startTime - currentTime;
//                printf("randomAllocation:*****%f*****\n", timeLen);
                for(i = 0; i < hostList.size();i++) {
                    host = &hostList[i];
                    energy += host->runOneT(timeLen, is_obey_SLA);
                    host->setCurrentTime(task->startTime);
                }
                selTimes = 0;
                isObeySAL = false;
                while (!isAllocated) {
                    rand_Host_Index = (uint)(myRand() * (double)hostList.size());
                    host = &hostList[rand_Host_Index];
                    for(cpuIndex = 0; cpuIndex < host->getCpuNum(); cpuIndex++) {
//                        copy1 = *host;
                        isAllocated = host->allocateTo(*task,cpuIndex,&freqs[0]);
                        if(isAllocated) break;
                    }
                    if(selTimes > 1000) {
                        slaCounter++;
                        isObeySAL = true;
                        break;
                    }
                    selTimes++;
                }
//                printf("randomAllocation:*****%d*****\n",rand_Host_Index);
                taskQueue.pop();
                currentTime += timeLen;
            }
            else {
                currentTime += timeLen;
                for(i = 0; i < hostList.size();i++) {
                    host = &hostList[i];
//                    printf("randomAllocation:*****%d*****\n", (int)host->taskList[0].size());
                    energy += host->runOneT(timeLen, is_obey_SLA);
                    host->setCurrentTime(currentTime);
                }
            }
        }

    }
    clock_t end = clock();
    printf("===>random allocation algorithm running time: %f s\n",(double)(end-start)/CLOCKS_PER_SEC);
    printf("===>SLAs : %d \n", slaCounter);
    return energy;
}

double sim_first_fit(vector<Host> &hostList, queue<Task> &taskQueue) {
    if(hostList.size() == 0 || taskQueue.size() == 0) return 0;

    double energy = 0.0;
    bool isFinished = false,isAllocated, isObeySAL = false;
    Task *task;
    Host *host;
    uint cpuIndex;
    double timeLen;
    double currentTime = 0.0;
    uint i;
    uint host_Index,slaCounter = 0;
    Host copy1;
    vector<double> freqs;
    host = &hostList[0];
    for(i = 0; i < host->getCpuNum(); i++) {
        freqs.push_back(host->freqLvls[0]);
    }
    bool is_obey_SLA;
    cout << ">>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<\n";
    clock_t start = clock();
    while(!isFinished) {
        is_obey_SLA = true;
        if(0 == taskQueue.size()) {
//            timeLen = getMinTimeInHosts(hostList);
            timeLen = 1.0;
            while(fabs(timeLen - 0.0) > 1e-10){
                timeLen = getMinTimeInHosts(hostList);
                currentTime += timeLen;
                for(i = 0; i < hostList.size(); i++) {
                    host = &hostList[i];
//                    printf("******%f %f******\n",host->cpuUtils[0],host->cpuUtils[1]);
                    energy = energy + host->runOneT(timeLen, is_obey_SLA);
                    host->setCurrentTime(currentTime);
                }
            }
            isFinished = true;
        }
        else{
            task = &taskQueue.front();
            timeLen = getMinTimeInHosts(hostList);
            if(fabs(timeLen - 0.0)< 1e-10) {
                timeLen = 1990063019910621.0;
            }
            if(task->startTime < timeLen + currentTime){
                isAllocated = false;
                timeLen = task->startTime - currentTime;
                for(i = 0; i < hostList.size(); i++) {
                    host = &hostList[i];
                    energy += host->runOneT(timeLen, is_obey_SLA);
                    host->setCurrentTime(task->startTime);
                }
                isObeySAL = false;
                host_Index = 0;
                while(!isAllocated) {
                    host = &hostList[host_Index];
                    for(cpuIndex = 0; cpuIndex < host->getCpuNum(); cpuIndex++) {
                        isAllocated = host->allocateTo(*task, cpuIndex, NULL);
                        if(isAllocated) {
                            host_Index = hostList.size();
                            break;
                        }
                    }
                    host_Index++;
                    if(host_Index == hostList.size()) {
                        slaCounter++;
                        break;
                    }
                }
                taskQueue.pop();

                currentTime += timeLen;
            }
            else {
                currentTime += timeLen;
                for(i = 0; i < hostList.size();i++) {
                    host = &hostList[i];
                    energy += host->runOneT(timeLen, is_obey_SLA);
                    host->setCurrentTime(currentTime);
                }
            }
        }
    }
    clock_t end = clock();
    printf("===>first fir algorithm running time: %f s\n",(double)(end-start)/CLOCKS_PER_SEC);
    printf("===>SLAs : %d \n", slaCounter);
    return energy;
}

double min_energy_change(Host *host, Task *task, int &cpuid, uint runtimes ,Local_Alg alg) {
    uint cpu_num = host->getCpuNum();
    double min_delta = DBL_MAX;
    vector<vector<double> > state;

    double e1, e2;
    Host host_copy = *host;
    bool is_obey_SLA = true;
    alg(&host_copy, runtimes, state, state);
    e1 = host_copy.run(state, is_obey_SLA);
    cpuid = -1;
    for(int i = 0; i < cpu_num; i++) {
        host_copy = *host;
        is_obey_SLA = true;
        if(host_copy.allocateTo(*task, i, NULL)) {
//printf("min_energy_change1: %f %d\n", task->remainTime, host_copy.getTaskNum());
            alg(&host_copy, runtimes, state, state);
            e2 = host_copy.run(state, is_obey_SLA);
//printf("***%f %f****\n", e1, e2);
            if(e2 - e1 < min_delta) {
                min_delta = e2 - e1;
                cpuid = i;
            }
        }
    }
    return min_delta;
}

double getMinTimeInHosts(vector<Host> hostList){
    if(hostList.size() == 0) return 0;
    double minTime = 1990063019910621.0;
    double hostMinRemainTime;
    Host *host;
    for(uint i = 0; i < hostList.size(); i++) {
        host = &hostList[i];
        hostMinRemainTime = host->getMinRemainTime();
        if(hostMinRemainTime < minTime) {
            minTime = hostMinRemainTime;
        }
    }
    if(minTime == 1990063019910621.0) {
        minTime = 0;
    }
    return minTime;
}

void random_alloc(vector<Host> &host_list, vector<Host_IP> &ip_list, Task &task) {
    uint host_num = host_list.size();
    uint try_times = 0, ran_index, cpu_index;
    is_task_allocated = false;
    Host *host;
    Host_IP *h_ip;
    double energy;
    while(!is_task_allocated) {
        ran_index = (uint)(myRand() * host_num);
        h_ip = &ip_list[ran_index];
        send_msg_to_host(h_ip, ABLE_TO_LOAD, &task, sizeof(Task));
        try_times++;
        if(try_times > MAX_TRY_TIMES) {
            break;
        }
        printf("----------is_task_allocated-------> %d\n", is_task_allocated);
    }
    if(!is_task_allocated) {
        // TODO
        return ;
    }
    else{
        send_msg_to_host(h_ip, TASK_ALLOCATE, NULL, 0);
    }
}

void first_fit(vector<Host> &host_list, vector<Host_IP> &ip_list, Task &task) {
    uint host_num = host_list.size();
    uint try_times = 0;
    is_task_allocated = false;
    Host *host;
    Host_IP *h_ip;
    double energy;
    for(int i = 0; i < host_num; i++) {
        h_ip = &ip_list[i];
        send_msg_to_host(h_ip, ABLE_TO_LOAD, &task, sizeof(Task));
        if(is_task_allocated) {
            break;
        }
    }
    if(!is_task_allocated) {
        // TODO
        return ;
    }
    else{
        send_msg_to_host(h_ip, TASK_ALLOCATE, NULL, 0);
    }
}

void my_alg(vector<Host> &host_list, vector<Host_IP> &ip_list, Task &task) {
    send_msg_to_all(ip_list, REQ_MIN_ENERGY, &task, sizeof(Task));
}