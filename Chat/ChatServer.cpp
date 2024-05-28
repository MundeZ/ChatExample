#include "ChatServer.h"
#include "User.h"

ChatServer::ChatServer(io_service& service, const ip::tcp::endpoint& endpoint)
    : acceptor_(service, endpoint), socket_(service) {
    connect_to_db();
    do_accept();
}

ChatServer::~ChatServer() { mysql_close(&mysql_); }

void ChatServer::do_accept() {
    acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
        if (!ec) {
            std::make_shared<User>(std::move(socket_),mysql_)->start();
        }
        do_accept();  // Accept the next connection
        });
}

void ChatServer::connect_to_db() {
    mysql_init(&mysql_);
    if (&mysql_ == nullptr) {
        Logger::instance().log("Error: can't create MySQL-descriptor");
        return;
    }

    if (!mysql_real_connect(&mysql_, "localhost", "root", "1234567890", "testdb", 0, NULL, 0)) {
        Logger::instance().log("Error: can't connect to database ");
    }
    else {
        Logger::instance().log("Successfully connected to the database!");
    }
}