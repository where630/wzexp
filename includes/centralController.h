#ifndef _H_CENTRAL_CONTROLLER_H_
#define _H_CENTRAL_CONTROLLER_H_

#include "myLib.h"
#include "allocation_Alg.h"

void *thread_send_msg(void *ptr);
//void send_msg(pthread_t &pid, Host_IP *host, Message *msg);
void send_msg_to_host(Host_IP *host_ip, char data_type, void *data, uint data_len);
void send_msg_to_all(vector<Host_IP> &hostList, char data_type, void *data, uint data_len);
void deal_msg_control(Host_IP *host, char *buf);

void controller_init();

void controller_main();
#endif