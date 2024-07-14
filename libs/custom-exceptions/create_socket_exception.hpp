#include <iostream>
#include <exception>

using namespace std;

class create_socket_exception:public exception{
public:
    create_socket_exception(){}

    const char* what() const noexcept override;
};