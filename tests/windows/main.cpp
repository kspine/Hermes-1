#include "Hermes.hpp"

using namespace hermes;
using namespace hermes::tools;
using namespace hermes::network;

void tests_tcp_socket(void) {
  tcp::socket socket;

  assert(socket.get_fd() == INVALID);
  assert(socket.get_host() == "");
  assert(socket.get_port() == 0);
}

int __cdecl main(void) {
  tests_tcp_socket();
  return 0;
}
