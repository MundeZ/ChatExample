#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <map>
#include <thread>
#include "mysql.h"
#include <mutex>
#include "Logger.h"
#include <sstream>
#include <memory>
#include <vector>
#include <algorithm>

using namespace boost::asio;

class User : public std::enable_shared_from_this<User> {
public:
    User(ip::tcp::socket socket, MYSQL& mysql, std::vector<std::shared_ptr<User>>& sessions);
    ~User();
    void start();

private:
    void do_read();
    void do_write(std::size_t length, std::map<std::string, std::string> responseMap);
    std::map<std::string, std::string> parseData(std::string json_data);
    void loginUser(MYSQL& mysql, const std::string& login, const std::string& password);
    void registrationUser(MYSQL& mysql, const std::string& login, const std::string& password);
    void findUser(MYSQL& mysql, const std::string& login);
    void sendMessage(const std::string& recipient);
    void menu(const std::map<std::string, std::string>& client_data);
    ip::tcp::socket socket_;
    MYSQL mysql_;
    std::mutex mutex_;

    static constexpr int max_length = 1024;
    char data_[max_length];

    std::vector<std::shared_ptr<User>>& sessions_;
};
