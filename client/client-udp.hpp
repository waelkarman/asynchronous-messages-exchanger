#pragma once
#include <iostream>
#include <functional>
#include <cstring>
#include <thread>
#include <atomic>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "datapacket.hpp"
#include "utils.hpp"
#include "safedequeue.hpp"
#include "safemap.hpp"

#define PORT 12345
#define BUFFER_SIZE 1024


/**
 * 
 *  Send n\                 / resend n
 *         \ wait for ack n/
 *          \
 *           \ send n+1
 * 
 * commento per ogni funzione
 *   aggiungere eccezioni per situazioni anomale 
 *   aggiungere CONDITION VARIABLE PER EVITARE INFINITE LOOP
 *   
 *     
 */

using namespace std;

class ClientUDP{
public:
    ClientUDP();

    void add_to_message_queue(const string& data);

    ~ClientUDP();

private:
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(server_addr);

    bool stop_condition;
    int sequence;
    atomic<int> packet_failure;
    int ms_send_interval;
    int ms_timeout_interval;
    TSMap<int,string> sent_messages;
    TSDeQueue<int> recv_ack_queue;
    TSDeQueue<string> messages_to_print;
    TSDeQueue<string> messages_to_send;
    vector<thread> workers;
    vector<function<void()>> tasks;
    mutex task_queue_mutex;
    MessageType TYPE_MSG = MSG;
    MessageType TYPE_ACK = ACK;
    datapacket dp;

    int timer(int s);

    void connection_status_monitor();

    void initialize();

    void main_loop();

    void deinitialize();

    void message_handler_loop();

    void fetch_and_send_loop(const int& ms_send_interval);

    void acknoledge_handling_loop();
    
    void received_message_loop();

    void task_launcher(vector<function<void()>> & tasks);
};


int main() {

    ClientUDP s;
    s.add_to_message_queue("CIAO");
    s.add_to_message_queue("HI");
    s.add_to_message_queue("SERVUS");
    s.add_to_message_queue("HALLO");
    s.add_to_message_queue("LIHAO");
    s.add_to_message_queue("HELLO");

    return 0;
}
