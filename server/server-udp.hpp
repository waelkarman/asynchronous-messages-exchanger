#pragma once
#include <iostream>
#include <functional>
#include <cstring>
#include <thread>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "datapacket.hpp"
#include "safequeue.hpp"
#include "safemap.hpp"
#include "utils.hpp"

#define PORT 12345
#define BUFFER_SIZE 1024

/**
 * 
 *  Send n\                 / resend n
 *         \ wait for ack n/
 *          \
 *           \ send n+1
 * 
 *  sinchonizza le strutture dati comuni ai 2 thread
 */

using namespace std;

class ServerUDP{
public:
    ServerUDP();

    void add_to_message_queue(const string& data, const int& n = 0);

    ~ServerUDP();

private:
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);

    bool client_address_available;
    bool stop_condition;
    int broken_pipe;
    size_t available;
    TSQueue<int> query;
    TSQueue<int> saved;
    TSQueue<int> ack_queue;
    TSQueue<string> recv_queue;
    TSMap<int,string> message_queue;
    vector<thread> workers;
    vector<function<void()>> tasks;
    int milliseconds;
    mutex task_queue_mutex;
    MessageType TYPE_MSG = MSG;
    MessageType TYPE_ACK = ACK;
    datapacket dp;

    void initialize();

    void main_loop();

    void deinitialize();

    void message_handler_loop();

    void fetch_and_send_loop(const int& milliseconds);

    void acknoledge_handling_loop();

    void received_message_loop();

    void task_launcher(vector<function<void()>> & tasks);
};


int main() {

    ServerUDP r;
    r.add_to_message_queue("DOG");
    r.add_to_message_queue("CAT");
    r.add_to_message_queue("ELEPHANT");
    r.add_to_message_queue("HORSE");
    r.add_to_message_queue("CAMEL");
    r.add_to_message_queue("TURTLE");
    
    return 0; 
}
