#include "broken_pipe_exception.hpp"

broken_pipe_exception::broken_pipe_exception(int packet_failure):failure(packet_failure){}

const char* broken_pipe_exception::what()const noexcept {
    stringstream ss;
    ss << "Fatal error: broken pipe. Packet Failure. " << failure << " pachets lost.";
    return ss.str().c_str();
}