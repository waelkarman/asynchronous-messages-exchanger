#include <iostream>
#include <exception>

using namespace std;

class recv_data_exception:public exception{
public:
    recv_data_exception(){}

    const char* what() const noexcept override;
};