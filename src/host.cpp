#include "../includes/host.h"


extern bool is_show_message;

Host::Host(uint hostID, uint cpuNum, uint coreNum, double alphaCPU, double pStatic, double pMax) {
    this->hostID  = hostID;
    this->cpuNum  = cpuNum;
    this->coreNum = coreNum;

    this->alphaCPU = alphaCPU;
    this->pStatic  = pStatic;
    this->pMax     = pMax;
    this->currentTime = 0;
    for(int i = 0; i < cpuNum; i++) {
        vector<Task> tl;
        taskList.push_back(tl);
        currCPUFreqs.push_back(0);
        cpuUtils.push_back(0);
    }
}

double Host::powerHost() {
//    double powerHost = pStatic;
    double powerHost = 0;
    double powerCPU = 0.0;
    double utilization = 0.0;
    int i = 0, j = 0;
    double freq = 0.0;
    int freqsLen = freqLvls.size();
    double fMax_3 = 1.0;
    double f_3 = 1.0;
    if (freqsLen == 0) {
        return 0.0;
    }
    fMax_3 = freqLvls[0] * freqLvls[0] * freqLvls[0];
    for (i = 0; i < cpuNum; i++) {
        utilization = cpuUtils[i];
        utilization /= this->coreNum;
        if (utilization > 1.0) {
            utilization = 1.0;
        }
        freq = this->currCPUFreqs[i];
        f_3 = freq * freq * freq;
        powerCPU += (utilization * (pMax - alphaCPU * (fMax_3 - f_3) * cpuNum  - pStatic)) / cpuNum;
 //       powerCPU += (utilization * (pMax - alphaCPU * (fMax_3 - f_3)  - pStatic));
    }
    powerHost += powerCPU;
//printf("***%f***\n", powerHost);
    return powerHost;

}


bool Host::allocateTo(Task task, uint cpuid, double *freqs) {

    if((cpuid > cpuNum) || ((task.cpuUtil + cpuUtils[cpuid]) / coreNum > upUtil)) {

        return false;
    }
    double cputime = 0;
    cputime = task.cpuTime;
    task.cpuTime = freqLvls[0] / currCPUFreqs[cpuid] * cputime;
    task.remainTime = task.cpuTime + task.ioTime;
    taskList[cpuid].push_back(task);
    cpuUtils[cpuid] += task.cpuUtil;
    sort(taskList[cpuid].begin(), taskList[cpuid].end());
//    printf("allocate: ******%u %f******\n", cpuid, task.cpuUtil);
    return true;
}

int Host::able_to_allocate(Task &task) {
    int cpuid;

    for(cpuid = 0; cpuid < cpuNum; cpuid++) {
//printf("===>%f %f<===\n", (task.cpuUtil + cpuUtils[cpuid]) / coreNum, upUtil);
        if(((task.cpuUtil + cpuUtils[cpuid]) / coreNum) < upUtil) {
            return cpuid;
        }
    }
    return -1;
}


void Host::changeOneFreq(int cpuid, double freq) {
    double cputime = 0;
    Task *task = NULL;
    int i = cpuid;
    for(int j = 0; j < taskList[i].size(); j++) {
        task = &taskList[i][j];
        cputime = task->cpuTime;
        task->cpuTime = cputime * currCPUFreqs[i] / freq;
        task->remainTime = task->cpuTime + task->ioTime;
    }
    currCPUFreqs[i] = freq;
}

void Host::changeFreqs(double *freqs) {
    double freq = 0;
    double cputime = 0;
    Task *task = NULL;
    for(int i = 0; i < cpuNum; i++) {
        freq = freqs[i];
        for(int j = 0; j < taskList[i].size(); j++) {
            task = &taskList[i][j];
            cputime = task->cpuTime;
            task->cpuTime = cputime * currCPUFreqs[i] / freq;
            task->remainTime = task->cpuTime + task->ioTime;
        }
        currCPUFreqs[i] = freq;
    }
}

int Host::removeTask(uint cpuid, Task *task) {
    if(task == NULL) {
        printf("Remove task error!===> task is NULL\n");
        return 0;
    }
    if(cpuid > cpuNum) {
        printf("Remove task error!===> cpuid > cpuNum\n");
        return 0;
    }
    Task *t;
    int i = 0;

    double remainTime, cpuTime, ioTime;
//    printf("%f %f %f\n", task->remainTime, cpuTime, ioTime);
    // for(i = 0; i < taskList[cpuid].size(); i++) {
    //     t = &taskList[cpuid][j];
    //     if (t->taskID == task->taskID) {
    //         vector<Task>::iterator iter = taskList[cpuid].begin() + i;
    //         taskList[cpuid].erase(iter);
    //         break;
    //     }
    // }
    t = &taskList[cpuid][0];
    if(is_show_message)
        printf("(%d ---> %f ---> %f)\n", task->taskID, task->runTime, task->deadline);
    if (t->taskID == task->taskID) {
        taskList[cpuid].erase(taskList[cpuid].begin());
    }
    else {
        printf("remove task error %d %d\n", t->taskID, task->taskID);
        return 0;
    }
    cpuUtils[cpuid] -= task->cpuUtil;
    if (cpuUtils[cpuid] < 0) {
        cpuUtils[cpuid] = 0;
    }
    return 1;
}

void Host::setCurrFreqs(double *freqs) {
    for(int i = 0; i < cpuNum; i++) {
        currCPUFreqs[i] = freqs[i];
    }
}

uint Host::getTaskNum() {
    uint taskNum = 0;
    for(int i = 0; i < cpuNum; i++) {
        taskNum += taskList[i].size();
    }
    return taskNum;
}

double Host::getMinRemainTime() {
    double minRemainTime = DBL_MAX;
    Task *task;
    for(int i = 0; i < cpuNum; i++) {
        if(taskList[i].size() > 0) {
            task = &taskList[i][0];
//            printf("***%f***\n",task->remainTime);
            if (task->remainTime < minRemainTime) {
                minRemainTime = task->remainTime;
            }
        }
    }
    return minRemainTime;
}

// |------------------------------------|
// |                                    |
// |                                    |
// |                                    |
// |                                    |
// |____________________________________|
double Host::runOneT(double timeLen, bool &is_obey_SLA) {
    this->currentTime += timeLen;
    //服务器处于空闲状态，这边需要重新考虑
    if(getTaskNum() == 0) {
//        return pStatic * timeLen;
        return 0;
    }

    double energy = 0.0;
    int cpuid = -1;
    double ioTime,cpuTime,remainTime = 0;
    Task *task = NULL,*taskFinish = NULL;
//    setCurrFreqs(&state[0][0]);
    energy = powerHost() * timeLen;
//    printf("runOneT:*****%f*****\n",powerHost());
    for(int i = 0; i < cpuNum; i++) {
        for(int j = 0; j < taskList[i].size(); j++) {
            task = &taskList[i][j];
            remainTime = task->remainTime;
            ioTime     = task->ioTime;
            cpuTime    = task->cpuTime;
            task->runTime += timeLen;
            if(task->runTime + task->startTime > task->deadline) {
//                printf(".-.-.-.-.-.-.-.-.-%d %f\n", task->taskID, task->runTime);
                is_obey_SLA = false;
            }
            if(fabs(remainTime - timeLen) < 1e-10){
                removeTask(i,task);
                j--;
            }
            else {
                task->remainTime = remainTime - timeLen;
                task->cpuTime    = task->remainTime * cpuTime / remainTime;
                task->ioTime     = task->remainTime * ioTime / remainTime;
                //这边与源代码不同，注意！！！！！！！！！
            }
        }
        sort(taskList[i].begin(),taskList[i].end());
    }
    return energy;
}

double Host::run(vector<vector<double> > &state, bool &is_obey_SLA) {
    int i = 0;
    double energy = 0.0;
    double minRemainTime = 0.0;
    double *freqs = NULL;
    // double tota_len = this->currentTime;
    while (getTaskNum() > 0) {
        if(i < state.size()) {
            freqs = &state[i][0];
            i++;

        }
        else {
            freqs = &currCPUFreqs[0];

        }
        changeFreqs(freqs);
        minRemainTime = getMinRemainTime();
//            printf("-------------%f------------\n", minRemainTime);
        energy += runOneT(minRemainTime, is_obey_SLA);
    }
//    printf("***%d %d***\n", taskList[0].size(), taskList[1].size());
    return energy;
}


Task* Host::find_task_by_id_cpuid(uint _cpuid, uint _taskid) {
    if(_cpuid > cpuNum) {
        return NULL;
    }
    for(int i = 0; i < taskList[_cpuid].size(); i++) {
        if(_taskid == taskList[_cpuid][i].taskID) {
            return &taskList[_cpuid][i];
        }
    }
    return NULL;
}

double Host::simulationRunOneT(double tiemLen) {
    double energy = 0.0;
    return energy;
}
double Host::simulationRun(vector<vector<double> > state) {
    double energy = 0.0;
    return energy;
}
/*double Host::powerCPU() {
    double powerCPU = 0.0;
    double utilization = 0.0;
    int i = 0, j = 0;
    double freq = 0.0;
    int freqsLen = freqLvls.size();
    double fMax_3 = 1.0;
    double f_3 = 1.0;
    if (freqsLen == 0) {
        return 0.0;
    }
    fMax_3 = freqLvls[freqsLen-1] * freqLvls[freqsLen-1] * freqLvls[freqsLen-1];
    for (i = 0; i < cpuNum; i++) {
        utilization = cpuUtils[i];
        utilization /= this->coreNum;
        if (utilization > 1.0) {
            utilization = 1.0;
        }
        freq = this->currCPUFreqs[i];
        f_3 = freq * freq * freq;
        powerCPU += (utilization * (pMax - alphaCPU * (fMax_3 - f_3) - pStatic));
    }
    return powerCPU;
}*/

/*void Host::taskInfluence(Task task, double *freqs, uint cpuid) {
    Task *t = NULL;
    double cputime = 0;
    for(int i = 0; i < cpuNum; i++) {
        for(int j = 0; j < taskList[i].size(); j++) {
            t = &taskList[i][j];
            cputime = t->cpuTime;
            t->cpuTime = currCPUFreqs[i] / freqs[i] *cputime;
            t->remainTime = t->cpuTime + t->ioTime;
        }
        sort(taskList[i].begin(),taskList[i].end());
    }
}*/
