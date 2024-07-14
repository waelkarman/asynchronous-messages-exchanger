#include <iostream>
#include <exception>

using namespace std;

class send_data_exception:public exception{
public:
    send_data_exception(){}

    const char* what() const noexcept override;
};