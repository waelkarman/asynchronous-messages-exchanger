#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <utils.hpp>

using namespace std;

class datapacket{
private:
    int seq;
    string data;
    char delimiter = ':';

public:
    datapacket();

    string pack(MessageType mp,const int& seq, const string& data){
        stringstream ss;
        ss << mp << ":" << seq << ":" << data;
        return ss.str();
    }

    std::vector<std::string> unpack(const string& s){
        std::vector<std::string> tokens;
        std::string token;
        std::stringstream ss(s);

        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }

        return tokens;
    }
};