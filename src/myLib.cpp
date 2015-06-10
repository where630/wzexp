#include "../includes/myLib.h"


double myRand() {

    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint seed = (uint)time(0) - (uint)tv.tv_usec;
//    printf("%u\n", seed);
    srand(seed);
    return (double)rand() / (double)RAND_MAX;
}

//返回正太分布随机数
//miu为平均值
//sigma为方差
double normalRand(const double miu, const double sigma) {
    double N = 12;
    double x = 0, temp = N;
//    myRand();
    do{
        x = 0;
        for(int i = 0; i < N; i++) {
            x = x +  myRand();
        }
        x = (x - temp / 2) / sqrt(temp / 12);
        x = miu + x * sqrt(sigma);
    } while( x <= 0.1 || x > 1.0);
    return x;
}

double readTasks(const char *filepath, uint task_num, queue<Task> &taskQueue){
    FILE *fp = fopen(filepath,"r");
    if(fp == NULL) {
        printf("\"myLib.cpp:readTask()\":FILE %s dose not exist!\n", filepath);
        return 0.0;
    }

    char line[256];
    fgets(line,256,fp);
    uint cpuId;
    char *p;
    int i = 0,counter = 0;
    double nums[5];
    double total_time = 0;
    while (fgets(line,256,fp) != NULL && counter < task_num) {
        i = 0;
        p = strtok(line,",");
        Task task;
        while(p != NULL) {
            nums[i] = (double)atof(p);
            p = strtok(NULL,",");
            i++;
        }
        task.taskID           = (uint)nums[0];
        task.startTime        = nums[1];
        task.deadline         = nums[2];
        task.userEstimateTime = nums[3];
        task.cpuUtil          = nums[4];

        //任务估计时间
        task.remainTime   = task.userEstimateTime;
        task.cpuTime      = task.remainTime * task.cpuUtil;
        task.ioTime       = task.remainTime - task.cpuTime;
        task.runTime      = 0.0;
        if(total_time < task.deadline) {
            total_time = task.deadline;
        }
        taskQueue.push(task);

        counter++;
    }
    return total_time;
//    printf("%d\n",(int)taskQueue.size());
}

vector<vector<double> > freqSpaces(vector<double> &freqLvls, vector<double> freqs, uint n, uint depth) {
    vector<vector<double> > result;
    if (depth == n) return result;
    vector<vector<double> > temp;
    if (depth < n) {
        for (uint i = 0; i < freqLvls.size(); i++) {
            vector<double> fs = freqs;
            fs.push_back(freqLvls[i]);
            if (depth == n-1) {
                result.push_back(fs);
            }
            else {
                temp = freqSpaces(freqLvls, fs, n, depth+1);
                result.insert(result.end(),temp.begin(), temp.end());
            }
        }
    }
    return result;
}

bool compare(const Task &t1, const Task &t2) {
    return t1.startTime < t2.startTime;
}

void generateTasks(const uint task_num, const double task_per_minute, double min_task_len, double max_task_len) {
    char line[128] = "taskid    startTime   deadline    estiamteTime    utilizations";
    FILE *fp = fopen("allTask.txt", "w");
    if(fp == NULL) {
        printf("myLib.cpp::generateTasks():File open error!\n");
        exit(0);
    }
    fprintf(fp, "%s\n", line);
    double task_len;
    int i;
    vector<Task> v;
    for(i = 0; i < task_num; i++) {
        Task task;
        task_len              = min_task_len + (int)((max_task_len - min_task_len) * myRand() + 0.5);
        task.taskID           = i + 1;
        task.cpuUtil          = normalRand(0.75, 1);
        task.startTime        = (int)((myRand() * task_num / task_per_minute * 60) + 0.5 );
        task.deadline         = task.startTime + task_len * 1.5;
        task.userEstimateTime = task_len;
        task.remainTime       = task_len;
        v.push_back(task);
    }
    sort(v.begin(), v.end(), compare);
    for(i = 0; i < task_num; i++) {
        fprintf(fp, "%u,%f,%f,%f,%f\n", v[i].taskID, v[i].startTime, v[i].deadline, v[i].userEstimateTime, v[i].cpuUtil);
    }
    fclose(fp);

}

void displayV1(vector<double> &v1) {
    int i;
    printf("begin:----------myLib.cpp:displayV1()----------\n");
    for(i = 0; i < v1.size(); i++) {
        printf("%f ",v1[i]);

    }
    printf("\nend:----------myLib.cpp:displayV1()----------\n");
}


void displayV2(vector<vector<double> > &v2) {
    int i,j;
    printf("begin:----------myLib.cpp:displayV2()----------\n");
    for(i = 0; i < v2.size(); i++) {
        for(j = 0; j < v2[i].size(); j++) {
            printf("%f ",v2[i][j]);
        }
        printf("\n");
    }
    printf("end:----------myLib.cpp:displayV2()----------\n");
}

/*void bindCPU(uint id, pthread_t pth) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(id, &mask);

    if(pthread_setaffinity_np(pth, sizeof(mask), &mask) < 0 ) {
        printf("===>set thread affinity failed!\n");
        return;
    }
}
*/

int writeFile(const char* filepath, const char* mode, const char* content) {
    FILE *fp = fopen(filepath, mode);
    if(NULL == fp) {
        printf("===>Error: file open error %s\n", filepath);
        return -1;
    }

    fprintf(fp, "%s", content);
    fclose(fp);
}

