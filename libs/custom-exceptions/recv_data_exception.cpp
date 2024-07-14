#include "recv_data_exception.hpp"

const char* recv_data_exception::what() const noexcept {
    return "Error receiving data.";
}
