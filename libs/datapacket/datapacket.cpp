#include "datapacket.hpp"

datapacket::datapacket(){}

string datapacket::pack(MessageType mp,const int& seq, const string& data){
    stringstream ss;
    ss << mp << ":" << seq << ":" << data;
    return ss.str();
}

vector<string> datapacket::unpack(const string& s){
    vector<string> tokens;
    string token;
    stringstream ss(s);

    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}