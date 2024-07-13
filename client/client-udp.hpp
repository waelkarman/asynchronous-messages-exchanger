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
#include "utils.hpp"
#include "safequeue.hpp"
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
 *  fix quando non ricevi mai risposta
 *   const ed altri attributi utili spazio attoglio gli uguale e chicche di stile
 *   aggiungere eccezioni per situazioni anomale 
 *   aggiungere CONDITION VARIABLE PER EVITARE INFINITE LOOP
 *      rimpiazza la queue con una deque
 *     
 */

using namespace std;

class ClientUDP{
public:
    ClientUDP();

    void add_to_message_queue(const string& data, const int& n = 0);

    ~ClientUDP();


private:
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(server_addr);

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

    ClientUDP s;
    s.add_to_message_queue("CIAO",0);
    s.add_to_message_queue("HI",1);
    s.add_to_message_queue("SERVUS",2);
    s.add_to_message_queue("HALLO",3);
    s.add_to_message_queue("LIHAO",4);
    s.add_to_message_queue("HELLO",5);

    return 0;
}
