#include "server-udp.hpp"

ServerUDP::ServerUDP():available(0),broken_pipe(0),milliseconds(1000),stop_condition(false),client_address_available(false){
    initialize();
    main_loop();
}

ServerUDP::~ServerUDP(){
    deinitialize();
}

void ServerUDP::add_to_message_queue(const string& data, const int& n){
    if(n==0){
        message_queue.insert(available,data);
        query.push(available);
        available++;
    }else{
        message_queue.insert(n,data);
        query.push(n);
    }
}

void ServerUDP::initialize(){
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "Errore nella creazione del socket" << endl;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Errore nel binding del socket" << endl;
        close(sockfd);
    }
}

void ServerUDP::main_loop(){
    
    function<void()> task_message_handler_loop = [this]() {
        this->message_handler_loop();
    };
    function<void()> task_fetch_and_send_loop = [this](){
        this->fetch_and_send_loop(this->milliseconds);
    };
    function<void()> task_acknoledge_handling_loop = [this]() {
        this->acknoledge_handling_loop();
    };
    function<void()> task_received_message_loop = [this]() {
        this->received_message_loop();
    };

    tasks.push_back(move(task_message_handler_loop));
    tasks.push_back(move(task_fetch_and_send_loop));
    tasks.push_back(move(task_acknoledge_handling_loop));
    tasks.push_back(move(task_received_message_loop));

    while(tasks.size()>0){
        workers.push_back(thread([this](vector<function<void()>> & tasks){this->task_launcher(tasks);},ref(tasks)));
    }
}


void ServerUDP::deinitialize(){
    for(thread& w : workers){
        if(w.joinable()){
            w.join();
        }
    }
    close(sockfd);
}

void ServerUDP::message_handler_loop(){
    while(true){
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        cout << "Ricevuto messaggio da " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << endl;
        if (n < 0) {
            cerr << " Errore nella ricezione dei dati" << endl;
            close(sockfd);
        }else{
            client_address_available = true;
        }
        buffer[n] = '\0';
        string s(buffer);
        vector<string> pack = dp.unpack(s);

        switch (stoi(pack[0])) {
            case MSG:
                cout<<"il messaggio è un MSG"<<endl;
                recv_queue.push(s);
                break;
            case ACK:
                cout<<"il messaggio è un ACK"<<endl;
                ack_queue.push(static_cast<int>(stoi(pack[1])));
                break;
            default:
                cout<<"messaggio non categorizzato"<<endl;
        }
    }
}

void ServerUDP::fetch_and_send_loop(const int& milliseconds){
    while (!client_address_available){
        this_thread::sleep_for(chrono::milliseconds(2000));
    }
    int key=0;
    string pack;
    while(!stop_condition){
        
        if(!query.empty()){
            query.printQueue();
            key = query.front();
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
        
        int n = sendto(sockfd, pack.c_str(), strlen(pack.c_str()), 0, (const struct sockaddr *)&client_addr, addr_len);
        if (n < 0) {
            cerr << "Errore nell'invio dei dati" << endl;
            close(sockfd);
        }

        this_thread::sleep_for(chrono::milliseconds(milliseconds));
    }
}

void ServerUDP::acknoledge_handling_loop(){
    int key_stored=0;
    bool missing=false;
    while(!stop_condition){

        if(!ack_queue.empty()){
            int key_received = ack_queue.front();
            ack_queue.pop();
            
            if( key_received >= key_stored){
                if( (key_received-key_stored) > 0 && message_queue.find(key_stored)){
                    cout << " chiedo il rinvio di "<< key_stored << endl;
                    query.push(key_stored);
                    cout << " salvo il valore fra i ricevuti non processati "<< key_received << endl;
                    saved.push(key_received); 
                }else if(message_queue.find(key_stored)){
                    cout << "ack "<< key_stored <<  " ricevuto, cancello i dati associati " << endl;
                    message_queue.erase(key_stored);
                    key_stored++;

                    while(!saved.empty() && !missing){
                        cout << " Vedi se la lista dei non processati contiene qualcosa" << endl;
                        if(saved.front() == key_stored){
                            cout << " RECUPERO "<< saved.front() <<" il mancante dalla lista" << endl;
                            message_queue.erase(key_stored);
                            saved.pop();
                            key_stored++;
                        }else{
                            cout << " mi fermo a recuperare perche manca "<< key_stored << endl;
                            query.push(key_stored);
                            missing=true;
                        }
                    }
                    
                    missing=false;
                }
                
                if(!message_queue.find(key_stored)&& !message_queue.empty()){
                    cout << " IMPOSSIBILE RECUPERARE I DATI PER IL NUMERO DI SEQUENZA "<< key_stored << endl; 
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

void ServerUDP::received_message_loop(){
    // bool first = true;
    while (true) {
        if(!recv_queue.empty()){
            vector<string> content = dp.unpack(recv_queue.front());
            recv_queue.pop();
            
            cout << "ricevuto  " << content[2] << " invio  ack: "<< content[1] << endl; 

            // if(first){
            //     cout << "SALTO"<<endl;
            //     first=false;
            //     continue;
            // }

            string pack = dp.pack(TYPE_ACK,stoi(content[1]),"ACK_MESSAGE");
            sendto(sockfd, pack.c_str(), strlen(pack.c_str()), 0, (const struct sockaddr *)&client_addr, addr_len);           
        }
    }
}

void ServerUDP::task_launcher(vector<function<void()>> & tasks){
    while(!tasks.empty()){
        function<void()> f;
        {
            lock_guard<mutex> lock(task_queue_mutex);
            
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