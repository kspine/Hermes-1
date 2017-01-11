#define CATCH_CONFIG_MAIN
#include <chrono>
#include <memory>
#include <thread>
#include "Hermes.hpp"
#include "catch.hpp"

using namespace hermes;
using namespace hermes::network;

//
// Workers tests section
//
SCENARIO("testing workers (Thread pool)") {
  WHEN("giving 3 jobs to process") {
    hermes::tools::workers workers(3);

    REQUIRE(workers.are_working());
    workers.enqueue_job([]() {});
    workers.enqueue_job([]() {});
    workers.enqueue_job([]() {});
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    workers.enqueue_job([&workers]() {
      workers.stop();
      REQUIRE(!workers.are_working());
    });
  }
}

//
// TCP socket tests sections
//
SCENARIO("testing tcp socket default constructor") {
  tcp::socket socket;
  tcp::socket socket2;

  REQUIRE(socket.get_fd() == -1);
  REQUIRE(socket.get_host() == "");
  REQUIRE(socket.get_port() == 0);
  REQUIRE(socket == socket2);
}

SCENARIO("testing tcp socket: server operations") {
  GIVEN("default tcp sockets") {
    tcp::socket default_socket;
    tcp::socket socket_for_test;

    WHEN("assigning a name to the socket (bind)") {
      REQUIRE(socket_for_test == default_socket);
      REQUIRE_NOTHROW(socket_for_test.bind("127.0.0.1", 27017));
      REQUIRE(socket_for_test.get_fd() != -1);
      REQUIRE(socket_for_test.get_host() == "127.0.0.1");
      REQUIRE(socket_for_test.get_port() == 27017);
      REQUIRE(socket_for_test.is_socket_bound() == true);
      REQUIRE((socket_for_test == default_socket) == false);
      REQUIRE_THROWS(socket_for_test.bind("127.0.0.1", 27017));
      REQUIRE_NOTHROW(socket_for_test.close());
    }

    WHEN("marking the socket as passive (listen)") {
      REQUIRE_THROWS(default_socket.listen(30));
      REQUIRE_NOTHROW(socket_for_test.bind("127.0.0.1", 27017));
      REQUIRE_NOTHROW(socket_for_test.listen());
      REQUIRE_NOTHROW(socket_for_test.close());
    }

    WHEN("accepting a new connection") {
      REQUIRE_THROWS(default_socket.accept());

      std::thread server([&socket_for_test]() {
        REQUIRE_NOTHROW(socket_for_test.bind("127.0.0.1", 27017));
        REQUIRE_NOTHROW(socket_for_test.listen());
        auto client = std::make_shared<tcp::socket>(socket_for_test.accept());
        REQUIRE((socket_for_test == *client) == false);
        REQUIRE(client->get_fd() != socket_for_test.get_fd());
        REQUIRE_NOTHROW(socket_for_test.close());
      });

      std::this_thread::sleep_for(std::chrono::seconds(1));

      std::thread client([&default_socket]() {
        REQUIRE_NOTHROW(default_socket.connect("127.0.0.1", 27017));
        REQUIRE_NOTHROW(default_socket.close());
      });

      REQUIRE_NOTHROW(server.join());
      REQUIRE_NOTHROW(client.join());
    }
  }
}

SCENARIO("testing tcp socket: client operations") {
  GIVEN("default tcp sockets") {
    tcp::socket default_socket;
    tcp::socket socket_for_test;

    WHEN("connecting to the given endpoint") {
      std::thread server([&default_socket]() {
        REQUIRE_NOTHROW(default_socket.bind("127.0.0.1", 27017));
        REQUIRE_NOTHROW(default_socket.listen());
        auto client = std::make_shared<tcp::socket>(default_socket.accept());
        REQUIRE_NOTHROW(default_socket.close());
      });

      std::this_thread::sleep_for(std::chrono::seconds(1));

      std::thread client([&socket_for_test]() {
        REQUIRE_NOTHROW(socket_for_test.connect("127.0.0.1", 27017));
        REQUIRE_NOTHROW(socket_for_test.close());
      });

      REQUIRE_NOTHROW(server.join());
      REQUIRE_NOTHROW(client.join());
    }

    WHEN("sending/receiving data") {
      std::thread server([&default_socket]() {
        REQUIRE_NOTHROW(default_socket.bind("127.0.0.1", 27017));
        REQUIRE_NOTHROW(default_socket.listen());
        auto client = std::make_shared<tcp::socket>(default_socket.accept());
        std::string rcv(client->receive().data());
        REQUIRE(rcv == "test ok :)");
        REQUIRE(rcv.size() == 10);
        REQUIRE_NOTHROW(default_socket.close());
      });

      std::this_thread::sleep_for(std::chrono::seconds(1));

      std::thread client([&socket_for_test]() {
        REQUIRE_NOTHROW(socket_for_test.connect("127.0.0.1", 27017));
        auto bytes = socket_for_test.send("test ok :)");
        REQUIRE(bytes == 10);
        REQUIRE_NOTHROW(socket_for_test.close());
      });

      REQUIRE_NOTHROW(server.join());
      REQUIRE_NOTHROW(client.join());
    }
  }
}

//
// TCP client tests section
//
SCENARIO("testing tcp client") {
  WHEN("using default constructor and move constructor") {
    tools::TIMEOUT = 0;

    tcp::client client;

    REQUIRE(!client.is_connected());
    set_poller(nullptr);
  }
}

//
// TCP server tests section
//
SCENARIO("testing tcp server") {
  WHEN("constructing a tcp server") {
    tools::TIMEOUT = 0;

    tcp::server server;

    REQUIRE(!server.is_running());
    set_poller(nullptr);
  }
}

//
// Poll poller tests section
//
SCENARIO("testing poller") {
  WHEN("testing basic features") {
    tools::TIMEOUT = 0;

    REQUIRE(poller_g == nullptr);
    std::shared_ptr<poller> poller;
    poller = get_poller();
    REQUIRE(poller != nullptr);

    tcp::socket socket;
    REQUIRE(poller->has<tcp::socket>(socket) == false);

    poller->add<tcp::socket>(socket);
    REQUIRE(poller->has<tcp::socket>(socket) == true);

    poller->remove<tcp::socket>(socket);
    REQUIRE(poller->has<tcp::socket>(socket) == false);
  }
}
