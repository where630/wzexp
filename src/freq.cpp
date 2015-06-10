#include "../includes/freq.h"
#include <unistd.h>

using namespace std;

void getFreqLvls(uint cpuid, vector<double> &cpu_freq_lvls) {
	uint i = 0;
	double freq = 0;
	char path[FILE_PATH_LEN],buf[256];
	char ch = '.';
	char *f = NULL;
	sprintf(path, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies",cpuid);
	FILE *fp = fopen(path,"r");
	if(fp == NULL) {
		printf("\"freq.cpp:getFreqLvls()\":Please check the file path!\n");
	}
	fgets(buf,256,fp);
	while(ch != '\0') {
		if(ch == ' ') {
			freqLvlNum++;
		}
		i++;
		ch = buf[i];
	}
	buf[i-1] = '\0';
//	printf("***%d***\n", freqLvlNum);
	f = strtok(buf, " ");
	i = 0;
	while(f != NULL) {
		freq = atof(f) / NUMBER_GHZ;
		cpu_freq_lvls.push_back(freq);
		f = strtok(NULL," ");
//		printf("%d\n",freq);
		i++;
	}
    cpu_freq_lvls.erase(cpu_freq_lvls.begin());
}

int setGovernor(uint cpuid, uint cpuNum, uint sameCPUs, const char* governor) {
	uint i = 0, coreIndex = 0;
	char path[FILE_PATH_LEN];
	FILE *fp = NULL;
	for(i = 0; i < sameCPUs; i++) {
		coreIndex = cpuid + cpuNum * i;
		sprintf(path, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", coreIndex);
//		printf("%s\n",path);
		fp = fopen(path,"w");
		if(fp == NULL) {
			printf("\"freq.cpp:setGovernor()\":File cannot open, please check the permission file path!\n");
			return 0;
		}
		fprintf(fp, "%s\n",governor);
		fclose(fp);
	}
	return 1;
}

int change_governor(string governor) {
    uint total_cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    uint same_freq_num = cpus_with_same_freq();
    uint cpu_num = total_cpu_num / same_freq_num;

    for(int i = 0; i < cpu_num; i++) {
        setGovernor(i, cpu_num, same_freq_num, governor.c_str());
    }
}

int setFrequency(uint cpuid, uint cpuNum, uint sameCPUs, uint freq) {
	uint i = 0, coreIndex = 0;
	char path[FILE_PATH_LEN];
	FILE *fp = NULL;
	for(i = 0; i < sameCPUs; i++) {
		coreIndex = cpuid + cpuNum * i;
		sprintf(path, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_setspeed", coreIndex);
//		printf("%s\n",path);
		fp = fopen(path, "w");
		if(fp == NULL) {
//			printf("\"freq.cpp:setFrequency()\":File cannot open, please check the permission or file path!\n");
			return 0;
		}
		fprintf(fp, "%d\n",freq);
		fclose(fp);
	}
	printf("Set frequency %u successfully of cpu %d\n", freq, cpuid);
	return 1;
}

void get_cpu_times(char *buf, vector<vector<unsigned long> > &cpu_time, uint cpu_num, vector<unsigned long> &total) {
	FILE *fp = fopen("/proc/stat", "r");
    if(NULL == fp) {
        printf("===> Open file /proc/stat failed.\n");
        exit(0);
    }
    fgets(buf, MAX_BUF_SIZE, fp);
    unsigned long local_total;
    for(int i = 0; i < cpu_num; i++) {
        fgets(buf, MAX_BUF_SIZE, fp);
    	char *p;
		p = strtok(buf, " ");
	    p = buf + strlen(p) + 1;
	    vector<unsigned long> v;
	    v.push_back(strtoul(p, &p, 0));		//user
	    v.push_back(strtoul(p, &p, 0));		//system
	    v.push_back(strtoul(p, &p, 0));		//nice
	    v.push_back(strtoul(p, &p, 0));		//idle
	    v.push_back(strtoul(p, &p, 0));		//iowait
	    v.push_back(strtoul(p, &p, 0));		//irq
	    v.push_back(strtoul(p, &p, 0));		//softirq
//        printf("cpu%d : %lu %lu %lu %lu %lu %lu %lu ", i, v[0], v[1],v[2],v[3],v[4],v[5],v[6]);

	    local_total = 0;
	    for(int j = 0; j < v.size(); j++) {
	    	local_total += v[j];
	    }
//        printf("cpu%d : %lu %lu\n", i, v[3], local_total);
	    total.push_back(local_total);
	    cpu_time.push_back(v);
    }
    fclose(fp);
}

void get_cpu_utils(vector<double> &cpu_utils) {
    char buf[MAX_BUF_SIZE], *p;
    uint cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    uint i;
    cpu_utils.erase(cpu_utils.begin(), cpu_utils.end());
    vector<vector<unsigned long> > cpu_time1, cpu_time2;
    vector<unsigned long> total1, total2;
    get_cpu_times(buf, cpu_time1, cpu_num, total1);
    sleep(1);
    get_cpu_times(buf, cpu_time2, cpu_num, total2);
    double cpu_util;
    for(i = 0; i < cpu_num; i++) {
    	cpu_util = 1 - (double)(cpu_time2[i][3] - cpu_time1[i][3]) / (double)(total2[i] - total1[i]);
    	cpu_utils.push_back(cpu_util);
//        printf("cpu utilization of cpu%d is %f\n", i, cpu_util);
    }
}

void get_cpu_freqs(vector<double> &cpu_freqs){
    char command[] = "grep \"cpu MHz\" /proc/cpuinfo | cut -f2 -d:";
    char buf[16];
    uint cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    FILE *fp = popen(command, "r");
    if(NULL == fp) {
        printf("===> Get current frequency error : command to analysis /proc/cpuinfo error!\n");
        exit(0);
    }
    double freq;
    cpu_freqs.erase(cpu_freqs.begin(), cpu_freqs.end());
    for(int i = 0; i < cpu_num; i++) {
        fgets(buf, 16, fp);
        freq = atof(buf) / NUMBER_GHZ * 1000.0;
        cpu_freqs.push_back(freq);
//        printf("cpu frequency of cpu%d is %f GHz\n", i, freq);
    }
    fclose(fp);
}

uint cpus_with_same_freq(){
	char command[] = "cpufreq-info | grep \"CPUs which run\"";
	FILE *fp = popen(command, "r");
	if(NULL == fp) {
		printf("===> Get CPUs which runs at same frequency failed!\n");
		exit(0);
	}
	char buf[128];
	fgets(buf, 128, fp);
	char *p = strtok(buf, ":");
	p = buf + strlen(p) + 2;
	char *q = strtok(p, " ");
	uint cpus_at_same_freq = 0;
	while(q != NULL) {
		cpus_at_same_freq++;
		q = strtok(NULL, " ");
	}
//	printf("Number of CPUs which runs at same frequency is: %u\n", cpus_at_same_freq);
    fclose(fp);
	return cpus_at_same_freq;
}