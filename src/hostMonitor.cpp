#include "../includes/hostMonitor.h"
#include <iostream>

using namespace std;

vector<Real_Task> rt_list(max_task_num);
vector<pthread_t> pid_list(max_task_num);
Task allocated_task;
int allocated_cpuid;
vector<vector<double> > state;
int sa_times = 0;
int vds_times = 0;
int cpus_at_same_freq = 1;
char file_content[2048];

void generate_random_task();
void update_task_pointer(vector<Real_Task> &rt_list);

void thread_host() {
    host_init();
    int sockfd, client_fd, on = 1;
    struct sockaddr_in my_addr;
    struct sockaddr_in remote_addr;
    //创建套接字
    vector<vector<double> > fs;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket create failed!");
        exit(1);
    }
    int retval = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    //绑定端口地址
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(SERVER_PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero), 8);
    if (bind(sockfd, (struct sockaddr*) &my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind error!");
//        exit(1);
    }

    //监听端口
    if(listen(sockfd, BACKLOG) == -1) {
        printf("%s\n", "listen error");
        exit(1);
    }
    while (1) {
        unsigned int sin_size = sizeof(struct sockaddr_in);
        if ((client_fd = accept(sockfd, (struct sockaddr*) &remote_addr,&sin_size)) == -1) {
            perror("accept error!");
            continue;
        }
        printf("Received a connection from %s\n", (char*) inet_ntoa(
                remote_addr.sin_addr));

        //接受client发送的请示信息
        int rval;
        char buf[MAX_BUF_SIZE];
        if ((rval = read(client_fd, buf, MAX_BUF_SIZE)) < 0) {
            perror("reading stream error!");
            continue;
        }

        int index = deal_msg_host(buf);
        if(index != -1) {
            if(local_alg_id == ALG_SA) {
                alg_SA(&host, sa_times, state, state);
            }
            else if(local_alg_id == ALG_VDS) {
                alg_VDS(&host, vds_times, state, state);
            }
            if(state.size() != 0) {
                host.changeFreqs(&state[0][0]);
                for(int i = 0; i < host.getCpuNum(); i++) {
                    setFrequency(i, host.getCpuNum(), cpus_at_same_freq, (uint)(state[0][i] * NUMBER_GHZ));
                }
            }
            if(pthread_create(&pid_list[index], NULL, run_real_task, &rt_list[index]) != 0) {
                printf("===>create real task failed");
            }

            displayV2(state);
        }
        //向client发送信息
        if(!fork()) {

            if(send(client_fd, buf, MAX_BUF_SIZE, 0) == -1) {
                perror("send error!\n");
            }
            close(client_fd);
            exit(0);
        }
        close(client_fd);
    }
    close(sockfd);
}


int deal_msg_host(char *buf) {
    //TODO
    int index = -1;
    if(NULL == buf) {
        printf("===>recive message error!\n");
        return -1;
    }
    char data_type = buf[0];
    switch(data_type) {
        case REQ_HOST_INFO: {
            buf[0] = RESP_HOST_INFO;
            uint *phid = (uint *)(buf + 1);
            host.setHostID(*phid);
            struct timeval tv_now;
            gettimeofday(&tv_now, NULL);
            double currentTime = tv_now.tv_sec + tv_now.tv_usec / 1000000.0;
            host.setCurrentTime(currentTime);
            uint *pu_data = (uint *)&host;
            double *pd_data = (double *)(pu_data + 4);
            uint udl = sizeof(uint) * (UINT_DATA_IN_HOST + 1);
            memcpy(buf+1, pu_data, udl);
            memcpy(buf+udl+1, pd_data, sizeof(double) * DOUB_DATA_IN_HOST);
            printf("===>DATA_TYPE::REQ_HOST_INFO--->currentTime : %f\n", pd_data[4]);
            break;
        }
        case REQ_FREQ_LVLS: {
            buf[0] = RESP_FREQ_LVLS;
            uint size = host.freqLvls.size();
            double *pd_freq_lvls = &host.freqLvls[0];
            memcpy(buf+1, &size, sizeof(uint));
            memcpy(buf+1+sizeof(uint), pd_freq_lvls, sizeof(double) * size);
            break;
        }
        case ABLE_TO_LOAD: {

            buf[0] = RESP_ABLE_TO_LOAD;
            allocated_cpuid = -1;
            memcpy(&allocated_task, buf+1, sizeof(Task));
            allocated_cpuid = host.able_to_allocate(allocated_task);
            memcpy(buf+1, &allocated_cpuid, sizeof(int));
//            printf("===>ABLE_TO_LOAD: cpuid---> %d\n", allocated_cpuid);
            break;
        }
        case REQ_MIN_ENERGY: {
            printf("----------REQ_MIN_ENERGY----------\n");
            buf[0] = RESP_MIN_ENERGY;

            memcpy(&allocated_task, buf+1, sizeof(Task));
            Task *task = &allocated_task;
//            uint cpuid;
//printf("-----1:%f-----\n", task->remainTime);
            double energy;
            if(local_alg_id == ALG_SA) {
                energy = min_energy_change(&host, task, allocated_cpuid, sa_times, alg_SA);
            }
            else if(local_alg_id == ALG_VDS) {
                energy = min_energy_change(&host, task, allocated_cpuid, vds_times, alg_VDS);
            }
            memcpy(buf+1, &energy, sizeof(double));
            memcpy(buf+sizeof(double)+1, &allocated_cpuid, sizeof(uint));
//printf("-----2:%f-----\n", task->remainTime);
            printf("==>min_energy_change: %f\n", energy);
            break;
        }
        case REQ_CPU_INFO: {
            uint cpu_num = host.getCpuNum();
            get_cpu_utils(host.cpuUtils);
            get_cpu_freqs(host.currCPUFreqs);
            buf[0] = RESP_CPU_INFO;
            uint len = sizeof(double) * host.currCPUFreqs.size();
            memcpy(buf+1, &host.currCPUFreqs[0], len);
            memcpy(buf+1+len, &host.cpuUtils[0], len);
            printf("===>Host--->DATA_TYPE::REQ_CPU_INFO: send cpu info\n");
            break;
        }
        case TASK_ALLOCATE: {
            buf[0] = RESP_TASK_ALLOCATE;
            host.allocateTo(allocated_task, allocated_cpuid, NULL);
            struct timeval tv_now;
            gettimeofday(&tv_now, NULL);
/*            double time_now = tv_now.tv_sec + tv_now.tv_usec / 1000000.0;
            allocated_task.startTime = time_now;*/
            Real_Task *prt;

            pthread_mutex_lock(&mutex);
            for(int i = 0; i < max_task_num; i++) {
                if(rt_list[i].id == -1) {
                    index = i;
                    break;
                }
            }

            time_t current_time;
            time(&current_time);
            sprintf(file_content, "   >>>Task %d is allocate to cpu %d at %s\n", allocated_task.taskID, allocated_cpuid, ctime(&current_time));
            writeFile("hlog.txt", "a", file_content);
            prt = &rt_list[index];
            prt->cpuid = allocated_cpuid;
            prt->id = index;
            int cpuNum = host.getCpuNum();
            host.allocateTo(allocated_task, allocated_cpuid, NULL);
            prt->taskid = allocated_task.taskID;
            update_task_pointer(rt_list);
//            printf("==================%f\n", allocated_task.remainTime);
//             for(int i =0; i < cpuNum; i++) {
//                 for(int j = 0; j < host.taskList[i].size(); j++) {
//                     if(host.taskList[i][j].taskID == allocated_task.taskID) {
//                         rt->taskid = rt->task->taskID;
//                         break;
//                     }
//                 }
//             }
            pthread_mutex_unlock(&mutex);
            break;
        }
        case REQ_HOST_INIT: {
            buf[0] = RESP_HOST_INIT;
            host_init();
            break;
        }
        default: {
            printf("===>Host: default case\n");
        }
    }
    return index;
}



void host_init() {
    struct timeval tv_start;
    gettimeofday(&tv_start, NULL);
    double currentTime = tv_start.tv_sec + tv_start.tv_usec / 1000000.0;
    local_alg_id = 0;
    int is_hyper_threading;
    cout << "   >>>Enter cpus with same frequency: ";
    cin >> cpus_at_same_freq;
    while(local_alg_id <=0 || local_alg_id >2) {
        cout << "   >>>Enter local algorithm (1: SA; 2: VDS): ";
        cin >> local_alg_id;
    }
    cout << "   >>>Enter max steps:";
    if(local_alg_id == ALG_SA)
        cin >> sa_times;
    else
        cin >> vds_times;

    uint core_num = sysconf(_SC_NPROCESSORS_ONLN);
//    uint cups_at_same_freqs = cpus_with_same_freq();
    uint cpu_num = core_num / cpus_at_same_freq;
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
    for(i = 0; i < max_task_num; i++) {
        rt_list[i].id = -1;
    }

    writeFile("hlog.txt", "w", file_content);
//    thread_host();
}

void update_task_pointer(vector<Real_Task> &rt_list) {
    for(int i = 0; i < max_task_num; i++) {
        if(rt_list[i].id != -1) {
            rt_list[i].ptask = host.find_task_by_id_cpuid(rt_list[i].cpuid, rt_list[i].taskid);
            if(NULL == rt_list[i].ptask) {
                printf("===>Cannot find task %d in cpu %d", rt_list[i].cpuid, rt_list[i].taskid);
                rt_list[i].id = -1;
            }
        }
    }
}

void host_test() {
    int task_num;
    int cpu_num_of_host;
    cout << "Please input CPU numbers:";
    cin >> cpu_num_of_host;
    cout << "Please input task numbers:";
    cin >> task_num;
    vector<Task> v_task(task_num);
    vector<Real_Task> v_real_task(task_num);
    double freqs[cpu_num_of_host];
    double utils[cpu_num_of_host];
    int cpuids[cpu_num_of_host];

    char buf[128];
    FILE *fp = fopen("hostTest.txt", "r");

    char *p;
    if(fp == NULL) {
        cout << "File open error\n";
        return;
    }
    int i = 0;
    host_init();
    for(i = 0; i < cpu_num_of_host; i++) {
        host.cpuUtils[i] = 0;
    }
    i = 0;
    generate_random_task();
    while(!feof(fp) && i < task_num) {
        fgets(buf, 256, fp);
        cout<<buf;
        p = strtok(buf, ",");
        cpuids[i] = atoi(p);
        p = strtok(NULL, ",");
        freqs[i] = atof(p);
        p = strtok(NULL, ",");
        utils[i] = atof(p);

        v_task[i].cpuUtil = utils[i];
        v_task[i].taskID = i + 1;
        v_task[i].remainTime = 60.0;
        v_real_task[i].cpuid = cpuids[i];
        v_real_task[i].ptask = &v_task[i];
        host.allocateTo(v_task[i], i, NULL);

        if(setFrequency(cpuids[i], cpu_num_of_host, 1, (uint)(freqs[i] * NUMBER_GHZ))) {
            host.changeOneFreq(cpuids[i], freqs[i]);
        }
        if(!fork()) {
            pthread_t pid;
            if(pthread_create(&pid, NULL, run_real_task, &v_real_task[i]) < 0) {
                cout << "Thread create error in host_test\n";
            }
            pthread_join(pid, NULL);
            exit(0);
        }
        i++;
    }
    printf("..........Power: %f %f %f\n", host.getPStatic(), host.powerHost(), host.getPStatic() + host.powerHost());
}

void set_all_same_freq() {
    double freq;
    int cpu_num_of_host;
    cout << "Please input CPU numbers:";
    cin >> cpu_num_of_host;
    cout << "Please input frequency:";
    cin >> freq;

    uint cpus_at_same_freq = cpus_with_same_freq();

    for(int i=0; i < cpu_num_of_host; i++) {
        setFrequency(i, cpu_num_of_host, cpus_at_same_freq, (uint)(freq * NUMBER_GHZ));
    }
}

void generate_random_task() {
    int cpuid, freq_index;
    double util, freq;
    FILE *fp = fopen("hostTest.txt", "w");
    for(int i = 0; i < host.getCpuNum(); i++) {
        util = myRand();
        freq_index = (int)(myRand() * (double)host.freqLvls.size());
        freq = host.freqLvls[freq_index];
        fprintf(fp, "%d,%f,%f\n", i, freq, util);
    }
    fclose(fp);
}

void *run_real_task(void *pd) {
    Real_Task *prt = (Real_Task *)pd;
    if(-1 == prt->id) {
        printf("===>Error: No such real task\n");
        return NULL;
    }
    uint cpuid = prt->cpuid;
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpuid, &mask);
    if(sched_setaffinity(0, sizeof(mask), &mask) < 0) {
        printf("===>set thread affinity failed!\n");
    }
    struct timeval tvstart, tvend, tvnow;

    double use_time, run_time;
    double start_time, now_time;
    gettimeofday(&tvstart, NULL);
    start_time = tvstart.tv_sec + tvstart.tv_usec / 1000000.0;

    prt->ptask->startTime = start_time;
    uint sleep_time;
    uint busy_time = (uint)(prt->ptask->cpuUtil * 100);
    sleep_time = 100 - busy_time;

    gettimeofday(&tvstart, NULL);
    use_time = 0;
    int counter = 0;
    while(fabs(prt->ptask->remainTime - 0) > 1.0) {
        while (use_time < busy_time) {
            gettimeofday(&tvend, NULL);
            use_time = 1000000 * (tvend.tv_sec - tvstart.tv_sec) + (tvend.tv_usec - tvstart.tv_usec);
            use_time /= 1000;
        }
        usleep(sleep_time * 1000);
        counter = (counter + 1) % 10;
        if(counter == 0) {
            pthread_mutex_lock(&mutex);
            prt->ptask->remainTime -= 1;
            prt->ptask->runTime += 1;
            pthread_mutex_unlock(&mutex);
        }
        use_time = 0.0;
        gettimeofday(&tvstart, NULL);
    }

    pthread_mutex_lock(&mutex);

    time_t current_time;
    time(&current_time);
    sprintf(file_content, "   !!!Task %d is finished at %s\n", prt->ptask->taskID, ctime(&current_time));
    writeFile("hlog.txt", "a", file_content);

    host.removeTask(cpuid, prt->ptask);
    prt->id = -1;
    update_task_pointer(rt_list);
    printf("task num is %d\n", host.getTaskNum());
    if(local_alg_id == ALG_SA) {
        alg_SA(&host, sa_times, state, state);
    }
    else if(local_alg_id == ALG_VDS) {
        alg_VDS(&host, vds_times, state, state);
    }
    if(state.size() != 0) {
        for(int i = 0; i < host.getCpuNum(); i++) {
            setFrequency(i, host.getCpuNum(), cpus_at_same_freq, (uint)(state[0][i] * NUMBER_GHZ));
        }
        host.changeFreqs(&state[0][0]);
    }

    pthread_mutex_unlock(&mutex);
}

int run_one_task(char *task_num, int freq, int coreid, int utilization) {
    FILE *fp = fopen("running_tasks.txt", "a");

    if (fp == NULL) {
        printf("   File open error in function: run_one_task()");
        return 0;
    }
    time_t _time = time(NULL);
    struct tm *tm_local = localtime(&_time);

    //format local time
    char str_f_t[100];
    strftime(str_f_t, 100, "%G-%m-%d %H:%M:%S", tm_local);

    //write local time and task to file
    fprintf(fp, "[%s] %s %d %d %d\n", str_f_t, task_name, freq, coreid, utilization);
    fclose(fp);

    //run task
    int status = system("sudo sh run_one_task.sh");
    return status;
}

