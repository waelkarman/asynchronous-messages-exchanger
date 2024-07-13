#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <utils.hpp>

using namespace std;

class datapacket{

public:
    datapacket();

    string pack(MessageType mp,int seq, string data){
        stringstream ss;
        ss << mp << ":" << seq << ":" << data;
        return ss.str();
    }

    std::vector<std::string> unpack(string s){
        std::vector<std::string> tokens;
        std::string token;
        std::stringstream ss(s);

        while (std::getline(ss, token, ':')) {
            tokens.push_back(token);
        }

        return tokens;
    }

private:
    int seq;
    string data;
    string delimiter = ":";
};


// int main(){

//     datapacket d;

//     string s = d.pack(5,"ciao");
    
//     std::vector<std::string> v = d.unpack(s);
//     cout << v[0] << " " << v[1];

// }