#ifndef _H_MYTYPE_H_
#define _H_MYTYPE_H_


#define MAX_CPUS                10
#define MAX_CORES               32
#define MAX_FREQ_LVLS           16

#define FILE_PATH_LEN           256
#define NUMBER_GHZ              1000000

#define MAX_BUF_SIZE            1024
#define SERVER_PORT             6666
#define MAX_BUF_SIZE            1024
#define BACKLOG                 10

#define IP_ADDR_LEN             16
#define MAX_HOST_NUMBER         16

#define UINT_DATA_IN_HOST       3
#define DOUB_DATA_IN_HOST       5
/*
*controller message types
*/
#define REQ_HOST_INFO           1
#define REQ_CPU_INFO            2
#define REQ_MIN_ENERGY          3
#define TASK_ALLOCATE           4
#define REQ_FREQ_LVLS           5
#define REQ_HOST_INIT           6
#define ABLE_TO_LOAD            7
/*
*host response message types
*/

#define RESP_HOST_INFO          20
#define RESP_CPU_INFO           21
#define RESP_MIN_ENERGY         22
#define RESP_FREQ_LVLS          23
#define RESP_ABLE_TO_LOAD       24
#define RESP_HOST_INIT          25
#define RESP_TASK_ALLOCATE      26

#define TIMEOUT                 100


#define MIN_TASK_LEN            60
#define MAX_TASK_LEN            120

#define DEFAULT_UP_UTIL         1
#define CPU_TIME_ITEM_LEN       7

#define MAX_TRY_TIMES           10

#define ALG_SA                  1
#define ALG_VDS                 2

#include <float.h>


typedef struct Model_Task{
    uint taskID;
    double startTime;
    double deadline;
    double userEstimateTime;

    double remainTime;
    double cpuTime;
    double ioTime;

    double cpuUtil;
    double runTime;

    bool operator < (const Model_Task &t) const {
        return remainTime < t.remainTime;
    }
 //   friend bool operator < (const struct Model_Task &t1, const struct Model_Task &t2);

}Task;

typedef struct Model_Host_IP{
    uint hostID;
    char *ip_addr;
    Model_Host_IP(uint id, char *addr) : hostID(id), ip_addr(addr){}
}Host_IP;

typedef struct Message{
    char data_type;
    char *data;
    uint data_len;
    Host_IP *host_ip;
}Msg;


typedef struct Struct_Real_Task {
    uint cpuid;
    int id;
    uint taskid;
    Task *ptask;
}Real_Task;


const int cmd_exit = 0;
const int cmd_host = 1;
const int cmd_controller = 2;
const int cmd_init = 3;
const int cmd_governor = 4;
const int cmd_sys_init = 5;
const int cmd_test = 6;
const int cmd_generate_tasks = 7;
const int cmd_sim_exp = 8;

const int max_task_num = 24;
#endif