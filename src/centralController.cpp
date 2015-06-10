#include "../includes/centralController.h"

extern bool is_task_allocated;

char content[2048];

void *thread_send_msg(void *ptr) {
//    printf("thread_send_msg\n");
//    Msg message;
//    memcpy(&message, ptr, sizeof(Msg));
    Msg *msg          = (Msg *)ptr;
    char data_type    = msg->data_type;
    char *data        = msg->data;
    Host_IP *host_ip  = msg->host_ip;
    char *serverIP    = host_ip->ip_addr;
    uint data_len     = msg->data_len;
    struct sockaddr_in sin;     //服务器的地址
    char buf[MAX_BUF_SIZE];
    char str[MAX_BUF_SIZE];
    int sfd;
    int port = SERVER_PORT;
    int flags;

    bzero((void *)&sin,sizeof(sin));
    sin.sin_family = AF_INET;   //使用IPV4地址族
    str[0] = data_type;
    if(data != NULL) {
        memcpy(str+1, data, data_len+1);
    }
    inet_pton(AF_INET,serverIP, (void *)&(sin.sin_addr));
    sin.sin_port = htons(port);

    sfd = socket(AF_INET, SOCK_STREAM, 0);

    /*flags = fcntl(sfd, F_GETFL, 0);
    if(flags < 0) {
        printf("===>flag error\n");
    }
    if(0 < fcntl(sfd, F_SETFL, flags | O_NONBLOCK)) {
        printf("===>set flag error\n");
    }*/

    if(-1 == connect(sfd,(struct sockaddr *)&(sin), sizeof(sin))) {
        printf("connect error\n");
        return NULL;
    }
    write(sfd, str, data_len+1);

    struct timeval timeout;
    fd_set fdr;
    FD_ZERO(&fdr);
    FD_SET(sfd, &fdr);

    timeout.tv_sec  = 10;
    timeout.tv_usec = 0;

    int retval = select(sfd+1, &fdr, NULL, NULL, &timeout);
    if(retval == -1) {
        printf("===>select error in func \"thread_send_msg\"\n");
        close(sfd);
        return NULL;
    }
    else if(retval == 0){
        printf("===>timeout\n");
        buf[0] = TIMEOUT;
        buf[1] = data_type;
        deal_msg_control(host_ip, buf);
        close(sfd);
        return NULL;
    }

 //   retval = ioctl(sfd, FIONBIO, (unsigned long*)&retval);
    read(sfd, buf, MAX_BUF_SIZE);

    deal_msg_control(host_ip, buf);
    close(sfd);
}

void deal_msg_control(Host_IP *host_ip, char *buf){
    char data_type = buf[0];

    //寻找目的主机
    uint hostID = host_ip->hostID;
    uint host_index;
    Host *host =  NULL;
    for(int i = 0; i < host_list.size(); i++) {
        if(hostID == host_list[i].getHostID()) {
            host       = &host_list[i];
            host_index = i;
            break;
        }
    }
    if(NULL == host) {
        printf("===> Cannot find host with hostID: %u\n", hostID);
        return;
    }
    switch(data_type) {
        case RESP_HOST_INFO: {
            uint *pu_data = (uint *)(buf + 1);
            double *pd_data = (double *)(pu_data + 4);

            //初始化host
            host->setCpuNum(pu_data[1]);
            host->setCoreNum(pu_data[2]);
            host->setUpUtil(pd_data[0]);
            host->setAlphaCPU(pd_data[1]);
            host->setPStatic(pd_data[2]);
            host->setPMax(pd_data[3]);
            host->setCurrentTime(pd_data[4]);
            printf("===> Receive response form host%u info successfully!\n", host->getHostID());
            printf("===>currentTime of hsot%u: %f\n", host->getHostID(), pd_data[4]);
            break;
        }
        case RESP_FREQ_LVLS: {
            uint *pu_freq_lvl_len = (uint *)(buf + 1);
            double *pd_freq_lvls  = (double *)(pu_freq_lvl_len + 1);
            host->freqLvls.erase(host->freqLvls.begin(), host->freqLvls.end());
            host->freqLvls.insert(host->freqLvls.begin(), pd_freq_lvls, pd_freq_lvls + *pu_freq_lvl_len);
            printf("===> Receive response from host%u freqLvls successfully!", host->getHostID());
            printf("===>freqLvls for host%d: \n", host->getHostID());
            for(int i = 0; i < host->freqLvls.size(); i++) {
                printf("%f ", host->freqLvls[i]);
            }
            printf("\n");
            break;
        }
        case RESP_CPU_INFO: {
            uint cpu_num = host->getCpuNum();
            double *freqs = (double *)(buf + 1);
            double *utils = (double *)(buf + 1 + sizeof(double) * cpu_num);
            host->currCPUFreqs.erase( host->currCPUFreqs.begin(),  host->currCPUFreqs.end());
            host->cpuUtils.erase(host->cpuUtils.begin(), host->cpuUtils.end());
            for(int i = 0; i < cpu_num; i++) {
                host->currCPUFreqs.push_back(freqs[i]);
                host->cpuUtils.push_back(utils[i]);
                printf("===>cpu%d--->freq: %f \t util: %f\n", i, host->currCPUFreqs[i], host->cpuUtils[i]);
            }
            break;
        }
        case RESP_MIN_ENERGY: {
            printf("--------RESP_MIN_ENERGY----------\n");
            double *energy = (double *)(buf+1);
            uint *cpuid = (uint *)(buf+sizeof(double)+1);
            int val = pthread_mutex_lock(&mutex);
            if(val != 0) {
                printf("error: host %d lock error!\n", hostID);
            }
            g_finished_host_num++;
            g_cpuid[host_index] = *cpuid;
            g_energy[host_index] = *energy;
            pthread_mutex_unlock(&mutex);
            break;
        }
        case RESP_ABLE_TO_LOAD: {
            int *cpuid = (int *)(buf + 1);
            if(*cpuid < 0) {
                is_task_allocated = false;
            }
            else {
                is_task_allocated = true;
            }
            printf("===>RESP_ABLE_TO_LOAD: %d %d\n", *cpuid, is_task_allocated);
            break;
        }
        case TIMEOUT: {
            if(buf[1] == REQ_MIN_ENERGY) {
                int val = pthread_mutex_lock(&mutex);
                if(val != 0) {
                    printf("error: host %d lock error!\n", hostID);
                }
                g_finished_host_num++;
                g_cpuid[host_index]  = -1;
                g_energy[host_index] = DBL_MAX;
                pthread_mutex_unlock(&mutex);
            }
            break;
        }
        case RESP_TASK_ALLOCATE: {
            printf("===>Task allocate\n");
            break;
        }
        case RESP_HOST_INIT: {
            printf("===>Host Initialized!\n");
            break;
        }
        default: {
            printf("===>Controller: default case\n");
        }
    }
}
void send_msg_to_host(Host_IP *host_ip, char data_type, void *data, uint data_len) {
    Message msg;
    msg.data_type = data_type;
    msg.data      = (char *)data;
    msg.data_len  = data_len;
    msg.host_ip   = host_ip;
    pthread_t pth;
    int retval = pthread_create(&pth, NULL, thread_send_msg, &msg);
    if(retval != 0) {
        printf("Message to %s failed.\n", host_ip->ip_addr);
    }
    pthread_join(pth, NULL);
}

void send_msg_to_all(vector<Host_IP> &hostList, char data_type, void *data, uint data_len) {

    int host_size = hostList.size();
    g_finished_host_num = 0;
    int i, retval;
    vector<pthread_t> pths(host_size);
    vector<Message> msgs(host_size);
    for (i = 0; i < host_size; i++) {
        msgs[i].data_type = data_type;
        msgs[i].data      = (char *)data;
        msgs[i].data_len  = data_len;
        msgs[i].host_ip   = &hostList[i];
        retval = pthread_create(&pths[i], NULL, thread_send_msg, &msgs[i]);
        if(retval != 0) {
            printf("Message to %s failed.\n", msgs[i].host_ip->ip_addr);
        }
    }

    for(i = 0; i < host_size; i++) {
        if(pthread_join(pths[i], NULL) != 0) {
            printf("Join error!\n");
        }
    }

    if(data_type == REQ_MIN_ENERGY) {
        printf("REQ_MIN_ENERGY\n");
        while(g_finished_host_num < host_size) {
            ;
        }
        double min_energy = DBL_MAX;
        uint host_index = -1;
        uint cpuid;
        for(i =0; i < host_size; i++) {
            if(g_energy[i] < min_energy && g_cpuid[i] != -1) {
                min_energy = g_energy[i];
                host_index = i;
                cpuid = g_cpuid[i];
            }
        }
        if(host_index != -1) {
            msgs[host_index].data_type = TASK_ALLOCATE;
            if(0 != pthread_create(&pths[host_index], NULL, thread_send_msg, &msgs[host_index])) {
                printf("===>send message error in Allocate Jobs\n");
            }
            pthread_join(pths[host_index], NULL);
        }
    }
}

void controller_init() {
    int host_num = -1;

    while(host_num <= 0) {
        cout << "Input host numbers: ";
        cin >> host_num;
    }
    host_ip_list.erase(host_ip_list.begin(), host_ip_list.end());
    host_list.erase(host_list.begin(), host_list.end());

    int i;
    for(i = 0; i < host_num; i++) {
        Host host(i+1);
        host_list.push_back(host);
        Host_IP host_ip(host.getHostID(), ip_addrs[i]);
        host_ip_list.push_back(host_ip);
    }
//    printf("%d %d\n", host_ip_list.size(), host_list.size());
//    send_msg_to_all(host_ip_list, REQ_HOST_INIT, NULL, 0);
    send_msg_to_all(host_ip_list, REQ_HOST_INFO, NULL, 0);
    send_msg_to_all(host_ip_list, REQ_FREQ_LVLS, NULL, 0);
    send_msg_to_all(host_ip_list, REQ_CPU_INFO, NULL, 0);

}

void controller_main() {
    controller_init();
    queue<Task> task_queue;

    int task_num = 0;
    cout << "   >>>Enter task numbers: ";
    cin >> task_num;

    struct timeval tv_start, tv_now;
    double time_start, time_now, time_go, time_end;
    time_end = readTasks("allTask.txt", task_num, task_queue);
    gettimeofday(&tv_start, NULL);
    time_t current_time;
    time(&current_time);
    time_start = tv_start.tv_sec + (double)(tv_start.tv_usec / 1000000.0);
    time_now = time_start;

    sprintf(content, ">>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<\nstart time: %s\n", ctime(&current_time));
    writeFile("clog.txt", "w", content);

    Task *task;
    int alg = -1;
    while(alg < 0 || alg > 2) {
        cout << "Input algorithm (0: random; 1: first fit; 2: kPP):";
        cin >> alg;
    }
    while(!task_queue.empty()) {
        task = &task_queue.front();
        time_go = time_now - time_start;
//        printf("............startTime..........time_go: %f %f\n", task->startTime, time_go);
        while(task->startTime - time_go <= 0.0) {
            //分配任务
            time(&current_time);
            cout << "while\n" << endl;
            sprintf(content, ">>> Allocate task %d at time: %s\n", task->taskID, ctime(&current_time));
            writeFile("clog.txt", "a", content);
            switch(alg) {
                case 0: {
                    random_alloc(host_list, host_ip_list, task_queue.front());
                    break;
                }
                case 1: {
                    first_fit(host_list, host_ip_list, task_queue.front());
                    break;
                }
                case 2: {
                    my_alg(host_list, host_ip_list, task_queue.front());
                    break;
                }
//                default : break;
            }

            task_queue.pop();
            if(task_queue.empty()) {
                break;
            }
            else{
                task = &task_queue.front();
            }
        }
        usleep(500000);
        gettimeofday(&tv_now, NULL);
        time_now = tv_now.tv_sec + (double)(tv_now.tv_usec / 1000000.0);
    }
    time_go = time_now - time_start;
    time_end += time_go;
    while(time_go < time_end) {
        sleep(1);
        gettimeofday(&tv_now, NULL);
        time_now = tv_now.tv_sec + (double)(tv_now.tv_usec / 1000000.0);
        time_go = time_now - time_start;
    }
    time(&current_time);
    sprintf(content, "end time: %s\n", ctime(&current_time));
    writeFile("clog.txt", "a", content);
}