#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <string>
#include "mysql.h"

#include "User.h"

using namespace boost::asio;

class ChatServer {
public:
    ChatServer(boost::asio::io_context& io_context, short port);
    ~ChatServer();
    void connect_to_db();

private:
    void do_accept();
    ip::tcp::acceptor acceptor_;
    ip::tcp::socket socket_;
    MYSQL mysql_;

    std::vector<std::shared_ptr<User>> sessions_;
};
