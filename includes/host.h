#ifndef _H_HOST_H_
#define _H_HOST_H_

#include <algorithm>
#include <stdio.h>
#include <math.h>
#include "myTypes.h"
#include <vector>

using namespace std;

class Host{
private:
    uint hostID;
    uint cpuNum;
    uint coreNum;

    double upUtil;
    double alphaCPU;
    double pStatic;
    double pMax;
    double currentTime;

public:
    vector<double> currCPUFreqs;
    vector<double> freqLvls;
    vector<double> cpuUtils;

    vector<vector<Task> > taskList;
    Host() : upUtil(DEFAULT_UP_UTIL) {};
    Host(uint id) : hostID(id) {};
//    Host(const Host& host);
    Host(uint hostID, uint cpuNum, uint coreNum, double alphaCPU, double pStatic, double pMax);
    double powerHost();
//    double powerCPU();
    int removeTask(uint cpuid,Task *task);
    bool allocateTo(Task task, uint cpuid, double *freqs);
//    void taskInfluence(Task task, double *freqs, uint cpuid);
    double runOneT(double timeLen, bool &is_obey_SLA);
    void changeOneFreq(int cpuid, double freq);
    void changeFreqs(double *freqs);
    double run(vector<vector<double> > &state, bool &is_obey_SLA);
    uint getTaskNum();
    double getMinRemainTime();
    double simulationRunOneT(double timeLen);
    double simulationRun(vector<vector<double> > state);
    int able_to_allocate(Task &task);
    Task *find_task_by_id_cpuid(uint _cpuid, uint _taskid);

    uint getHostID() {return hostID;}
    void setHostID(uint hostID) {this->hostID = hostID;}
    uint getCoreNum() {return coreNum;}
    void setCoreNum(uint coreNum) {this->coreNum = coreNum;}
    uint getCpuNum() {return cpuNum;}
    void setCpuNum(uint cpuNum) {this->cpuNum = cpuNum;}
    double getUpUtil() {return upUtil;}
    void setUpUtil(double upUtil) {this->upUtil = upUtil;}
//    void setHostID(double upUtil) {this->upUtil = upUtil;}
    double getAlphaCPU() {return alphaCPU;}
    void setAlphaCPU(double alphaCPU) {this->alphaCPU = alphaCPU;}
    double getPStatic() {return pStatic;}
    void setPStatic(double pStatic) {this->pStatic = pStatic;}
    double getPMax() {return pMax;}
    void setPMax(double pMax) {this->pMax = pMax;}
    double getCurrentTime() {return currentTime;}
    void setCurrentTime(double currentTime) {this->currentTime = currentTime;}

    void setCurrFreqs(double *freqs);

};
#endif