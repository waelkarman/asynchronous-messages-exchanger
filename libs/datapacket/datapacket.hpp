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

    string pack(MessageType mp,const int& seq, const string& data);

    vector<string> unpack(const string& s);
};