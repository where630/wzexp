#include "../includes/local_DVFS_Alg.h"


void alg_SA(Host *host, int maxSteps, vector<vector<double> > &state, vector<vector<double> > &freqSpace) {

    uint phaseNum = host->getTaskNum();

    if (phaseNum == 0) {
        return ;
    }
    state.erase(state.begin(), state.end());
    double *freqLvls = &host->freqLvls[0];
    uint cpuNum   = host->getCpuNum();
    uint freq_num = host->freqLvls.size();
    int i = 0, j = 0, t = 0;
    //设置最高频率
    for(i = 0; i < phaseNum; i++) {
        vector<double> v;
        for(j = 0; j < cpuNum; j++) {
            //最高频率的下标
            v.push_back(freqLvls[0]);
        }
        state.push_back(v);
    }
    bool is_obey_SLA = true;
    Host hostCopy = *host;
    double eMax = hostCopy.run(state, is_obey_SLA);
//    printf("===>alg_SA--->energy with max frequency: %f\n",eMax);
    double e = eMax;
    double et;
    double p,rand;
    double T = eMax * 0.2;
    double coolingRate = (double)((int)sqrt(T)*(int)sqrt(T)) / T;
    vector<double> preFreqs;
    clock_t start = clock();

    while (t < maxSteps) {
        is_obey_SLA = true;
        int k = (int)(myRand() * phaseNum);
        preFreqs = state[k];

        for(i = 0; i < cpuNum; i++) {
            j = (int)(myRand() * freq_num);
            state[k][i] = freqLvls[j];
        }
//        state[k] = freqSpace[(int)(myRand() * (double)freqSpace.size())];

        hostCopy = *host;
        et = hostCopy.run(state, is_obey_SLA);
//        printf("%f \n",et);
        p = exp(-(et - e) / T);
        rand = myRand();
        if ((et < e || rand < p) && is_obey_SLA) {
            e = et;
        }
        else{
            for(i = 0; i < cpuNum; i++) {
                state[k][i] = preFreqs[i];
            }
        }
        t++;
        T = T * coolingRate;
    }
    clock_t end = clock();
//    printf("local_DVFS_Alg.cpp:alg_SA(): running time: %f s\n",(double)(end-start)/CLOCKS_PER_SEC);
}

void alg_VDS(Host *host, int maxSteps, vector<vector<double> > &state, vector<vector<double> > &freqSpace) {
//    double *freqLvls = host->getFreqlvls();
    uint cpuNum   = host->getCpuNum();
    uint phaseNum = host->getTaskNum();
    uint freq_num = host->freqLvls.size();
    if (phaseNum == 0) {
        return;
    }
    state.erase(state.begin(), state.end());
    double *freqLvls = &host->freqLvls[0];
    int i = 0, j = 0, k = 0, t = 0, r;
    //设置选取空间
    uint subsize = 1;
    for(i = 0; i < cpuNum; i++) {
        subsize *= freq_num;
    }
    subsize = subsize > 40 ? 40 : subsize;
    //设置最高频率
    for(i = 0; i < phaseNum; i++) {
        vector<double> v;
        for(j = 0; j < cpuNum; j++) {
            //最高频率的下标
            v.push_back(freqLvls[0]);
        }
        state.push_back(v);
    }
    bool is_obey_SLA = true;
    Host hostCopy = *host;
    double eMax = hostCopy.run(state, is_obey_SLA);
//    cout << is_obey_SLA <<endl;
    double e = eMax;
    double et;
    vector<double> preFreqs;
    double p,rand;
  //  vector<vector<Task> > taskList = host->taskList;
//    uint *result = (uint*)malloc(subsize * sizeof(uint));
    clock_t start = clock();

    while (t < maxSteps) {
        is_obey_SLA = true;
        k = (int)(myRand() * phaseNum);
        preFreqs = state[k];
//        neighbor(subsize, freqSpace.size(), result);
        for(i = 0; i < subsize; i++) {
            for(j = 0; j < cpuNum; j++) {
                r = (int)(myRand() * freq_num);
                state[k][j] = freqLvls[r];
            }
//            state[k][j] = freqSpace[result[i]];
            hostCopy = *host;
//            hostCopy.taskList = taskList;
            et = hostCopy.run(state, is_obey_SLA);
//            printf("***%f %f***\n", et, e);
/*            if(!is_obey_SLA) {
//                printf("...... %d %d\n", t, i);
            }*/
            if (et < e && is_obey_SLA) {

                e = et;
                preFreqs = state[k];
            }
        }
        for(i = 0; i < cpuNum; i++) {
            state[k][i] = preFreqs[i];
        }
        t++;
    }
    clock_t end = clock();
//    printf("local_DVFS_Alg.cpp:alg_VDS(): running time: %f s\n",(double)(end-start)/CLOCKS_PER_SEC);
//    free(result);
}

/*
*在[0,big]中随机产生small个不同的数
*/
/*void neighbor(uint small,uint big, uint *result) {
//        int *result = new int[small];
        vector<uint> v;
        uint i = 0;
        for(i = 0; i < big; i++) {
            v.push_back(i);
        }
        srand((uint)time(0) - (uint)clock());
        random_shuffle(v.begin(), v.end());

        uint r = (uint) (big * myRand());
        for(i = 0; i < small; i++) {
            result[i] = v[(i + r) % big];
        }
//      for (i = 0; i < small; i++) {
//          System.out.println(result[i]);
//      }
//        return *result;
    }*/