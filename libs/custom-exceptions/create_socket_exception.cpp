#include "create_socket_exception.hpp"

const char* create_socket_exception::what() const noexcept {
    return "Socket creation error.";
}