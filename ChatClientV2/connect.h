#ifndef CONNECT_H
#define CONNECT_H

#include <iostream>
#include "boost/asio.hpp"
#include "boost/json.hpp"
#include "boost/system/error_code.hpp"
#include "map"
#include "qlineedit.h"
#include <QWidget>
#include <QDialog>

using namespace boost::asio;

enum Api {
    REGISTRATION,
    LOGIN,
    MESSAGE,
    FIND_USER
};

class Connect {
public:
    Connect();
    ~Connect();
    void connect();
    ip::tcp::socket& getSocket();
    std::string getApi(Api api);

    std::map<std::string, std::string> requestToServerRegistration(const std::string& api, const std::string& login = "", const std::string& password = "");
    std::map<std::string, std::string> requestToServerLogin(const std::string& api, const std::string& login = "", const std::string& password = "");
    std::map<std::string, std::string> requestToServerMessage(const std::string& api,const std::string& message = "", const std::string& recipient = "");
    std::map<std::string, std::string> requestToServerFindUser(const std::string& api, const std::string& login = "");

    std::map<std::string, std::string> responseFromServer();
private:
    io_service service_;
    ip::tcp::socket socket_;
    boost::system::error_code error_;
};

#endif // CONNECT_H
