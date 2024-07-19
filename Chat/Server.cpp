#include "Server.h"

Server::Server(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, ip::tcp::endpoint(ip::tcp::v4(), port)), socket_(io_context) {
    do_accept();
}

void Server::do_accept() {
    acceptor_.async_accept(socket_,
        [this](boost::system::error_code ec) {
            if (!ec) {
                std::make_shared<Session>(std::move(socket_), sessions_)->start();
            }
            do_accept(); // Снова вызываем do_accept() для приема новых соединений
        });
}
