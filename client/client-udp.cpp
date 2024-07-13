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
 *     splitta HEADER e CPP
 *      rimpiazza la queue con una deque
 *      verifica tutti i ritorni di funzione
 */

std::mutex task_queue_mutex;
class ClientUDP{
public:
    ClientUDP():available(0),broken_pipe(0),milliseconds(1000),stop_condition(false){
        initialize();
        main_loop();
    }

    void add_to_message_queue(const string& data, const int& n = 0){
        if(n==0){
            message_queue.insert(available,data);
            query.push(available);
            available++;
        }else{
            message_queue.insert(n,data);
            query.push(n);
        }
    }

    ~ClientUDP(){
        deinitialize();
    }


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
    vector<std::function<void()>> tasks;
    int milliseconds;
    MessageType TYPE_MSG = MSG;
    MessageType TYPE_ACK = ACK;
    datapacket dp;

    void initialize(){
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            std::cerr << "Errore nella creazione del socket" << std::endl;
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    }

    

    void main_loop(){
        
        std::function<void()> task_message_handler_loop = [this]() {
            this->message_handler_loop();
        };
        std::function<void()> task_fetch_and_send_loop = [this](){
            this->fetch_and_send_loop(this->milliseconds);
        };
        std::function<void()> task_acknoledge_handling_loop = [this]() {
            this->acknoledge_handling_loop();
        };
        std::function<void()> task_received_message_loop = [this]() {
            this->received_message_loop();
        };

        tasks.push_back(std::move(task_message_handler_loop));
        tasks.push_back(std::move(task_fetch_and_send_loop));
        tasks.push_back(std::move(task_acknoledge_handling_loop));
        tasks.push_back(std::move(task_received_message_loop));

        while(tasks.size()>0){
            workers.push_back(thread([this](vector<std::function<void()>> & tasks){this->task_launcher(tasks);},std::ref(tasks)));
        }
    }

    void deinitialize(){
        for(thread& w : workers){
            if(w.joinable()){
                w.join();
            }
        }
        close(sockfd);
    }

    void message_handler_loop(){
        while(true){
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
            cout << "Ricevuto messaggio da " << inet_ntoa(server_addr.sin_addr) << ":" << ntohs(server_addr.sin_port) << endl;
            if (n < 0) {
                std::cerr << " Errore nella ricezione dei dati" << std::endl;
                close(sockfd);
            }
            buffer[n] = '\0';
            string s(buffer);
            vector<string> pack = dp.unpack(s);

            switch (stoi(pack[0])) {
                case MSG:
                    cout<<"Il messaggio è un MSG."<<endl;
                    recv_queue.push(s);
                    break;
                case ACK:
                    cout<<"Il messaggio è un ACK. " << endl;
                    ack_queue.push(static_cast<int>(std::stoi(pack[1])));
                    break;
                default:
                    cout<<"messaggio non categorizzato"<<endl;
            }
        }
    }

    void fetch_and_send_loop(const int& milliseconds){
        string pack;
        while(!stop_condition){
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
            
            if(!query.empty()){
                query.printQueue();
                int key = query.front();
                cout << "send: invio valore estratto " << key << endl;
                query.pop();
                if(!message_queue.find(key)){ //fix accesso a dato non presente perche sono stati richiesti piu rinvii
                    continue;
                }
                pack = dp.pack(TYPE_MSG,key,message_queue.get(key));
            }else{
                cout << "send: invio valore di default -1" << endl;
                pack = dp.pack(TYPE_MSG,-1,"ALIVE");
            }
            
            int n = sendto(sockfd, pack.c_str(), strlen(pack.c_str()), 0, (const struct sockaddr *)&server_addr, addr_len);
            if (n < 0) {
                std::cerr << "Errore nell'invio dei dati" << std::endl;
                close(sockfd);
            }
        }
    }

    void acknoledge_handling_loop(){
        int key_stored=0;
        bool missing=false;
        while(!stop_condition){

            if(!ack_queue.empty()){
                int key_received = ack_queue.front();
                ack_queue.pop();
                
                if( key_received >= key_stored){
                    if( (key_received-key_stored) > 0 && message_queue.find(key_stored)){
                        cout << "chiedo il rinvio di "<< key_stored << endl;
                        query.push(key_stored);
                        cout << "salvo il valore fra i ricevuti non processati "<< key_received << endl;
                        saved.push(key_received);
                    }else if(message_queue.find(key_stored)){
                        cout << "ack "<< key_stored <<  " ricevuto, cancello i dati associati " << endl;
                        message_queue.erase(key_stored);
                        key_stored++;

                        while(!saved.empty() && !missing){
                            cout << "Vedi se la lista dei non processati contiene qualcosa" << endl;
                            if(saved.front() == key_stored){
                                cout << "RECUPERO  "<< saved.front() <<" il mancante dalla lista" << endl;
                                message_queue.erase(key_stored);
                                saved.pop();
                                key_stored++;
                            }
                            else{
                                cout << "mi fermo a recuperare perche manca "<< key_stored << endl;
                                query.push(key_stored);
                                missing=true;
                            }
                        }
                        
                        missing=false;
                    }
                    
                    if(!message_queue.find(key_stored) && !message_queue.empty()){
                        cout << "Si blocca la richiesta di rinvio di un dato che non è presente nella mappa."<< key_stored << endl; 
                        broken_pipe++;
                        if(broken_pipe>2){
                            cout << "fatal error: broken pipe. Transmission failure." << endl;
                            stop_condition=true;
                        }
                    }
                    
                }else{
                    if(key_received != -1){
                        cout<< "DATI ASSICIATO A "<< key_received << " GIA RICEVUTI"<<endl;
                    }else{
                        cout<< "Alive ack processed" << endl; 
                    }
                }

            }
        }
    }

    void received_message_loop(){
        while (true) {
            if(!recv_queue.empty()){
                vector<string> content = dp.unpack(recv_queue.front());
                recv_queue.pop();

                std::cout << "ricevuto  " << content[2] << " invio ack: "<< content[1] << endl; 
                
                string pack = dp.pack(TYPE_ACK,stoi(content[1]),"ACK_MESSAGE");
                sendto(sockfd, pack.c_str(), strlen(pack.c_str()), 0, (const struct sockaddr *)&server_addr, addr_len);            
            }
        }
    }

    void task_launcher(vector<std::function<void()>> & tasks){
        while(!tasks.empty()){
            std::function<void()> f;
            {
                std::lock_guard<std::mutex> lock(task_queue_mutex);
                
                if (tasks.empty()){
                    return;
                }
                f = tasks.back();
                tasks.pop_back();
                cout << "Thread loaded with task: " << tasks.size() << endl;
            }
            f();
        }
    }
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
