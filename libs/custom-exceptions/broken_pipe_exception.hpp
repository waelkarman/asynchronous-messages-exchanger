#include <iostream>
#include <sstream>
#include <exception>

using namespace std;

class broken_pipe_exception:public exception{
private:
    int failure; 
public:
    broken_pipe_exception(int packet_failure);

    const char* what() const noexcept override;
};