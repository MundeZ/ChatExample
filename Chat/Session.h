#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include <map>

using namespace boost::asio;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(ip::tcp::socket socket, std::vector<std::shared_ptr<Session>>& sessions);
    ~Session();
    void start();

private:
    void do_read();
    void do_write(std::size_t length, std::map<std::string, std::string> responseMap);
    void handle_disconnect();

    ip::tcp::socket socket_;
    static constexpr int max_length = 1024;
    char data_[max_length];
    std::vector<std::shared_ptr<Session>>& sessions_;
};

#endif // SESSION_HPP
