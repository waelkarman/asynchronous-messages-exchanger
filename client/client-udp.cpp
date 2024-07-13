#include <iostream>
#include <functional>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <datapacket.hpp>
#include <utils.hpp>
#include <timer.hpp>
#include <queue>
#include <map>
#include <thread>

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
 *  fix quando non ricevi mai risposta
 */

std::mutex task_queue_mutex;
class Sender{

public:
    Sender():key(0),available(0),key_stored(0),broken_pipe(0),milliseconds(1000),missing(false),stop_condition(false){}

    int initialize(){
        // Creazione del socket UDP
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            std::cerr << "Errore nella creazione del socket" << std::endl;
            return 1;
        }

        // Impostazione dell'indirizzo del server
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        return 0;
    }

    void add_to_message_queue(string data, int n = 0){
        if(n==0){
            message_queue[available]=data;
            query.push(available);
            available++;
        }else{
            message_queue[n]=data;
            query.push(n);
        }
    }

    void main_loop(){
        
        std::function<void()> task0 = [this](){
            this->fetch_and_send_loop(this->milliseconds);
        };
        std::function<void()> task1 = [this]() {
            this->acknoledge_handling_loop();
        };

        tasks.push_back(std::move(task0));
        tasks.push_back(std::move(task1));

        while(tasks.size()>0){
            workers.push_back(thread([this](vector<std::function<void()>> & tasks){this->th_lodable(tasks);},std::ref(tasks)));
        }
    }

    int deinitialize(){
        for(thread& w : workers){
            if(w.joinable()){
                w.join();
            }
        }
        close(sockfd);
        return 0;
    }

    ~Sender(){
        deinitialize();
    }


private:
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    string message = "Hello, Server";
    socklen_t addr_len = sizeof(server_addr);
    bool missing;
    bool stop_condition;
    int broken_pipe;
    size_t key;
    size_t available;
    size_t key_stored;
    std::queue<size_t> query;
    std::queue<size_t> saved;
    std::map<int,string> message_queue;
    std::mutex mtx;
    vector<thread> workers;
    vector<std::function<void()>> tasks;
    int milliseconds;
    MessageType TYPE = MSG;

    void printQueue(const std::queue<size_t>& q) {
        std::queue<size_t> tempQueue = q;
        std::cout << "Message Queue content: ";
        while (!tempQueue.empty()) {
            std::cout << tempQueue.front() << " ";
            tempQueue.pop();
        }
        std::cout << std::endl;
    }

    void fetch_and_send_loop(int milliseconds){
        while(!stop_condition){
            // message fetching
            datapacket dp;
            string pack;
            
            if(!query.empty()){
                printQueue(query);
                key = query.front();
                cout << "send: invio valore estratto " << key << endl;
                // Comment next line to test already received data. 
                query.pop();
                message = message_queue[key];
                pack = dp.pack(TYPE,key,message);
            }else{
                cout << "send: invio valore di default -1" << endl;
                pack = dp.pack(TYPE,-1,"MESSAGE_QUEUE EMPTY");
            }
            
            // send data
            int n = sendto(sockfd, pack.c_str(), strlen(pack.c_str()), 0, (const struct sockaddr *)&server_addr, addr_len);
            if (n < 0) {
                std::cerr << "Errore nell'invio dei dati" << std::endl;
                close(sockfd);
                //return 1;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        }
    }

    void acknoledge_handling_loop(){
        while(!stop_condition){
            // get answer from the server

            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
            if (n < 0) {
                std::cerr << "received: Errore nella ricezione dei dati" << std::endl;
                close(sockfd);
                //return 1;
            }
            buffer[n] = '\0';
            string s(buffer);
            size_t key_received = static_cast<size_t>(std::stoul(s));
            
            if( key_received >= key_stored){
                if( (key_received-key_stored) > 0 && message_queue.find(key_stored) != message_queue.end()){
                    cout << "received: chiedo il rinvio di "<< key_stored << endl;

                    query.push(key_stored);
                    cout << "received: salvo il valore fra i ricevuti non processati "<< key_stored << endl;
                    saved.push(key_received); 
                }else if(message_queue.find(key_stored) != message_queue.end()){
                    cout << "received: ricevo "<< key_stored <<  " e cancello i dati associati " << endl;
                    message_queue.erase(key_stored);
                    
                    while(!saved.empty() && !missing){
                        cout << "received: Vedi se la lista dei non processati contiene qualcosa" << endl;
                        if(saved.front() == key_stored+1){
                            cout << "received: RECUPERO il mancante dalla lista" << endl;
                            message_queue.erase(key_stored);
                            saved.pop();
                            key_stored++;
                        }else{
                            cout << "received: mi fermo a recuperare perche manca "<< key_stored << endl;
                            query.push(key_stored);
                            missing=true;
                        }
                    }
                    key_stored++;
                    missing=false;
                }
                
                std::cout << "received: Dati inviati al server: " << buffer << std::endl;

                if(message_queue.find(key_stored) == message_queue.end() && !message_queue.empty()){
                    cout << "received: IMPOSSIBILE RECUPERARE I DATI PER IL NUMERO DI SEQUENZA "<< key_stored << endl; 
                    broken_pipe++;
                    if(broken_pipe>2){
                        cout << "fatal error: broken pipe. Transmission failure." << endl;
                        stop_condition=true;
                    }
                }
                
            }else{
                cout<< "received: DATI ASSICIATO A "<< key_stored << " GIA RICEVUTI"<<endl;
            }

        }
    }

    void th_lodable(vector<std::function<void()>> & tasks){
        while(!tasks.empty()){
            std::function<void()> f;
            {
                std::lock_guard<std::mutex> lock(task_queue_mutex);
                
                if (tasks.empty()){
                    return;
                }
                f = tasks.back();
                tasks.pop_back();
                cout << "Thread " << " loaded with task: " << tasks.size() << endl;
            }
            f();
        }
    }

};



int main() {

    Sender s;
    s.add_to_message_queue("CIAO",0);
    s.add_to_message_queue("SERVUS",1);
    s.add_to_message_queue("HI",2);
    s.initialize();
    s.main_loop();
    s.add_to_message_queue("HALLO",3);
    s.add_to_message_queue("LIHAO",4);
    s.add_to_message_queue("HELLO",5);


    return 0;
}
