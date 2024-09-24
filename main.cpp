#include "tcp_handler.hpp"


int main(void) {
  try {
    tcp_handler("127.0.0.1");
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return -1;
  }
  return 0;
}
