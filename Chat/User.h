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

    std::string getLogin() const;
    void setLogin(const std::string& login);

private:
    void do_read();
    void do_write(std::map<std::string, std::string> responseMap);
    std::map<std::string, std::string> parseData(std::string json_data);
    void loginUser(const std::string& login, const std::string& password);
    void registrationUser(const std::string& login, const std::string& password);
    void findUser(const std::string& login);

    std::shared_ptr<User> findRecipient(const std::string& recipient);

    void sendMessage(const std::string& recipient, const std::string& message);

    void menu(const std::map<std::string, std::string>& client_data);

    ip::tcp::socket socket_;
    MYSQL mysql_;
    std::mutex mutex_;
    std::string login_;
    static constexpr int max_length = 1024;
    char data_[max_length];
    std::vector<std::shared_ptr<User>>& sessions_;
};
