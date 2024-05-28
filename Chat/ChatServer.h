#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include "mysql.h"

using namespace boost::asio;

class ChatServer
{
public:
    ChatServer(io_service& service, const ip::tcp::endpoint& endpoint);
    ~ChatServer();

private:
    void connect_to_db();
    void do_accept();
    ip::tcp::acceptor acceptor_;
    ip::tcp::socket socket_;
    MYSQL mysql_;
    

};

