#include "client-udp.hpp"

ClientUDP::ClientUDP():available(0),sequence(0),broken_pipe(0),milliseconds(1000),stop_condition(false){
    initialize();
    main_loop();
}

ClientUDP::~ClientUDP(){
    deinitialize();
}

void ClientUDP::add_to_message_queue(const string& data, const int& n){
    messages_to_send.push(data);
}

void ClientUDP::initialize(){
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "Errore nella creazione del socket" << endl;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
}

void ClientUDP::main_loop(){
        
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

void ClientUDP::deinitialize(){
    for(thread& w : workers){
        if(w.joinable()){
            w.join();
        }
    }
    close(sockfd);
}

void ClientUDP::message_handler_loop(){
    while(true){
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
        cout << "Messagge received from -> " << inet_ntoa(server_addr.sin_addr) << " : " << ntohs(server_addr.sin_port) << endl;
        if (n < 0) {
            cerr << " Errore nella ricezione dei dati" << endl;
            close(sockfd);
        }
        buffer[n] = '\0';
        string s(buffer);
        vector<string> pack = dp.unpack(s);

        switch (stoi(pack[0])) {
            case MSG:
                cout<<"Message type MSG."<<endl;
                messages_to_print.push(s);
                break;
            case ACK:
                cout<<"Message type ACK." << endl;
                recv_ack_queue.push(static_cast<int>(stoi(pack[1])));
                break;
            default:
                cout<<"Message type unknown."<<endl;
        }
    }
}


void ClientUDP::fetch_and_send_loop(const int& milliseconds){
    string pack;
    string data;
    while(!stop_condition){
        this_thread::sleep_for(chrono::milliseconds(milliseconds));
        
        if(!messages_to_send.empty()){
            data = messages_to_send.front();
            cout << "Sending: "<< data << " with   ack --> " << sequence << endl;
            pack = dp.pack(TYPE_MSG,sequence,data);
            messages_to_send.pop();
        }else{
            data = "ALIVE";
            cout << "Sending: "<< data << " with   ack --> " << sequence << endl;
            pack = dp.pack(TYPE_MSG,sequence,data);
        }
        
        int n = sendto(sockfd, pack.c_str(), strlen(pack.c_str()), 0, (const struct sockaddr *)&server_addr, addr_len);
        if (n < 0) {
            cerr << "Errore nell'invio dei dati" << endl;
            close(sockfd);
        }

        sent_messages.insert(sequence,data);

        tasks.push_back(move([this](){
            bool stop = false;
            while(!stop){
                int sec = timer(timer_done,sequence);
                
                if(sent_messages.find(sec)){
                    std::cout << "Timeout for " << sec << " resend .."<< endl;
                    string pack;
                    pack = dp.pack(TYPE_MSG,sec,sent_messages.get(sec));
                    sendto(sockfd, pack.c_str(), strlen(pack.c_str()), 0, (const struct sockaddr *)&server_addr, addr_len);
                }else{
                    stop = true;
                }
            }
        }));
        workers.push_back(thread([this](vector<function<void()>> & tasks){this->task_launcher(tasks);},ref(tasks)));

  
        sequence++;
    }
}
// lista da inviare consumata
// mappa messaggi inviati seuqenza valore 
// !QUALCUNO DEVE RIMUOVERE DA SENT MESSAGES


void ClientUDP::acknoledge_handling_loop(){
    while(!stop_condition){

        while(!recv_ack_queue.empty()){
            if(sent_messages.find(recv_ack_queue.front())){
                sent_messages.erase(recv_ack_queue.front());
                cout<< "ACK " << recv_ack_queue.front() << " received, message sent successfully."<<endl;
                recv_ack_queue.pop();
            }else{
                cout << "Duplicate ACK data already handled: " << recv_ack_queue.front() << endl;
                recv_ack_queue.pop();
            }

        }

        // handle the received failure sequence and out of order.
        
    }
}

void ClientUDP::received_message_loop(){
    while (true) {
        if(!messages_to_print.empty()){
            vector<string> content = dp.unpack(messages_to_print.front());
            messages_to_print.pop();

            cout << "Message received content " << content[2] << ",      send ack --> "<< content[1] << endl; 
            
            string pack = dp.pack(TYPE_ACK,stoi(content[1]),"ACK_MESSAGE");
            sendto(sockfd, pack.c_str(), strlen(pack.c_str()), 0, (const struct sockaddr *)&server_addr, addr_len);            
        }
    }
}

void ClientUDP::task_launcher(vector<function<void()>> & tasks){
    while(!tasks.empty()){
        function<void()> f;
        {
            lock_guard<mutex> lock(task_queue_mutex);
            
            if (tasks.empty()){
                return;
            }
            f = tasks.back();
            tasks.pop_back();
            cout << "A New thread is loaded with a new task." << endl;
        }
        f();
    }
}