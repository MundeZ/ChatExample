#include <iostream>
#include <boost/asio.hpp>
#include "ChatServer.h"

int main() {
    try {
        boost::asio::io_service service;
        ip::tcp::endpoint endpoint(ip::tcp::v4(), 1234);

        ChatServer server(service, endpoint);

        service.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
