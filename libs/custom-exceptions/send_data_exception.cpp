#include "send_data_exception.hpp"

const char* send_data_exception::what() const noexcept {
    return "Error sending data.";
}
