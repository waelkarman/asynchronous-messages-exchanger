#include "client-udp.hpp"

ClientUDP::ClientUDP():sequence(0),packet_failure(0),ms_send_interval(450),ms_timeout_interval(1000),stop_condition(false){
    initialize();
    main_loop();
}

ClientUDP::~ClientUDP(){
    deinitialize();
}

void ClientUDP::add_to_message_queue(const string& data){
    messages_to_send.push(data);
}

void ClientUDP::initialize(){
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "Socket create error." << endl;
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
        this->fetch_and_send_loop(this->ms_send_interval);
    };
    function<void()> task_acknoledge_handling_loop = [this]() {
        this->acknoledge_handling_loop();
    };
    function<void()> task_received_message_loop = [this]() {
        this->received_message_loop();
    };
    function<void()> task_connection_status_monitor = [this]() {
        this->connection_status_monitor();
    };

    tasks.push_back(move(task_message_handler_loop));
    tasks.push_back(move(task_fetch_and_send_loop));
    tasks.push_back(move(task_acknoledge_handling_loop));
    tasks.push_back(move(task_received_message_loop));
    tasks.push_back(move(task_connection_status_monitor));

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
    while(!stop_condition){
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
        cout << "Messagge received from -> " << inet_ntoa(server_addr.sin_addr) << " : " << ntohs(server_addr.sin_port) << endl;
        if (n < 0) {
            cerr << "Error receiving data." << endl;
            close(sockfd);
        }
        buffer[n] = '\0';
        string s(buffer);
        vector<string> pack = dp.unpack(s);

        switch (stoi(pack[0])) {
            case MSG:
                cout<<"Message type MSG."<<endl;
                messages_to_print.insert(stoi(pack[1]),pack[2]);
                break;
            case ACK:
                cout<<"Message type ACK." << endl;
                recv_ack_queue.push(stoi(pack[1]));
                break;
            default:
                cout<<"Message type unknown."<<endl;
        }
    }
}


void ClientUDP::fetch_and_send_loop(const int& ms_send_interval){
    string pack;
    string data;
    while(!stop_condition){
        this_thread::sleep_for(chrono::milliseconds(ms_send_interval));
        
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
            cerr << "Error sending data." << endl;
            close(sockfd);
        }

        sent_messages.insert(sequence,data);

        tasks.push_back(move([this](){
            bool stop = false;
            int retry = 0;
            while(!stop){
                int sec = timer(sequence);
                
                if(sent_messages.find(sec)){
                    retry++;
                    std::cout << "Timeout for " << sec << " resend .. ("<<retry<<")." << endl;
                    string pack;
                    pack = dp.pack(TYPE_MSG,sec,sent_messages.get(sec));
                    sendto(sockfd, pack.c_str(), strlen(pack.c_str()), 0, (const struct sockaddr *)&server_addr, addr_len);
                }else{
                    stop = true;
                }
                
                if(retry > 2){
                    std::cout << "ERROR: Packet "<<sec<<" lost."<< endl;
                    packet_failure++;
                    stop = true;
                }

            }
        }));
        workers.push_back(thread([this](vector<function<void()>> & tasks){this->task_launcher(tasks);},ref(tasks)));

  
        sequence++;
    }
}

void ClientUDP::acknoledge_handling_loop(){
    while(!stop_condition){

        while(!recv_ack_queue.empty()){
            if(sent_messages.find(recv_ack_queue.front())){
                sent_messages.erase(recv_ack_queue.front());
                cout<< "ACK " << recv_ack_queue.front() << " received, message sent successfully."<<endl;
                recv_ack_queue.pop();
            }else{
                cout << "ACK duplicated, data already received: " << recv_ack_queue.front() << endl;
                recv_ack_queue.pop();
            }

        }
    }
}

void ClientUDP::connection_status_monitor(){
    while(!stop_condition){
        if(packet_failure > 5){
            stop_condition = true;
            cout << "Fatal error: broken pipe. Packet Failure "<< packet_failure << endl;
        }else{
            cout << "Connection alive!." << endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(10)); 
    }
}

void ClientUDP::received_message_loop(){
    int size = 10;
    int message_processed = 0;
    vector<string> ordered_window(size);
    while (!stop_condition) {
        if(messages_to_print.find(message_processed)){
            ordered_window[static_cast<int>(message_processed % size)] = messages_to_print.get(message_processed);
            messages_to_print.erase(message_processed);
            
            string pack = dp.pack(TYPE_ACK,message_processed,"ACK_MESSAGE");
            sendto(sockfd, pack.c_str(), strlen(pack.c_str()), 0, (const struct sockaddr *)&server_addr, addr_len);       

            if(static_cast<int>(message_processed % size) == (size-1)){
                cout << "Received messages (ordered) ---------- >>";
                for_each(ordered_window.begin(),ordered_window.end(),[](string i){cout<< i << " ";});
                cout<<endl;  
            }
            message_processed++; 
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
            //cout << "A New thread is loaded with a new task." << endl;
        }
        f();
    }
}

int ClientUDP::timer(int s) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms_timeout_interval));
    return s;
}