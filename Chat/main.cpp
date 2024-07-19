#include <iostream>
#include <boost/asio.hpp>
#include "ChatServer.h"

int main() {
    setlocale(LC_ALL, "Russian");
    try {
        boost::asio::io_context io_context;
        ChatServer server(io_context, 8888);
        server.connect_to_db();
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}