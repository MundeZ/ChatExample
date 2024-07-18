#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <string>
#include "mysql.h"

using namespace boost::asio;

struct ClientInfo {
    std::string Login;
    ip::tcp::socket* userSocket;
};

class ChatServer {
public:
    ChatServer(io_service& service, const ip::tcp::endpoint& endpoint);
    ~ChatServer();

    void addToActiveClients(ClientInfo client);
    ip::tcp::socket* getFromActiveClients(const std::string& name);

private:
    void connect_to_db();
    void do_accept();
    ip::tcp::acceptor acceptor_;
    ip::tcp::socket socket_;
    MYSQL mysql_;
    std::vector<ClientInfo> activeClients_;
};
