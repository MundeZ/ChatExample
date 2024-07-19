#ifndef SERVER_HPP
#define SERVER_HPP

#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include "Session.h"

using namespace boost::asio;

class Server {
public:
    Server(boost::asio::io_context& io_context, short port);

private:
    void do_accept();

    ip::tcp::acceptor acceptor_;
    ip::tcp::socket socket_;
    std::vector<std::shared_ptr<Session>> sessions_;
};

#endif // SERVER_HPP
