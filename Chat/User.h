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

using namespace boost::asio;
class User : public std::enable_shared_from_this<User> {
public:
	User(ip::tcp::socket socket, MYSQL& mysql);
	~User();
	void start();

private:
	void response(const std::string& data);
	void registrationUser(MYSQL& mysql, const std::string& login, const std::string& password);
	void loginUser(MYSQL& mysql, const std::string& login, const std::string& password);
	void findUser(MYSQL& mysql, const std::string& login);
	void insertMessageIntoDB(MYSQL& mysql, const std::string& sender, const std::string& recipient, const std::string& message);
	void menu(const std::map<std::string, std::string>& client_data);
	std::map<std::string, std::string> get_data();
	ip::tcp::socket socket_;
	MYSQL mysql_;
	std::mutex mutex_;

};
