#include <iostream>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <map>
#include "mysql.h"



using namespace boost::asio;


bool findUser(MYSQL& mysql, const std::string& name)
{
    std::ostringstream queryStream;
    queryStream << "SELECT * FROM testdb.users WHERE name = '" << name << "'";
    std::string query = queryStream.str();
    if (mysql_query(&mysql, query.c_str()) != 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

std::string sendHandler(MYSQL& mysql, const std::string& from, const std::string& to, const std::string& message)
{

    // �������� �������� �� ��, ���������� �� ���� � ��, ���� ���, �� �� ���������� ���������.
    std::ostringstream queryStream;
    queryStream << "INSERT INTO testdb.messages(id, sender, recipient, message) values (default, '" << from << "', '" << to << "', '" << message << "')";
    std::string query = queryStream.str();
    if (mysql_query(&mysql, query.c_str()) != 0) {
        return "Error because: " + std::string(mysql_error(&mysql));
    }
    else {
        std::string ok = "Message inserted to db";
        return  ok;
    }
}

void sendMessage() // ��� ����� ������� ������ ��������� �� �� � �������� ������� ������������.
{
    //sendHandler();
}



std::string registrationUser(MYSQL& mysql, const std::string& login, const std::string& password)
{
    std::ostringstream queryStream;
    queryStream << "INSERT INTO testdb.users(id, name, password) values (default, '" << login << "', '" << password << "')";
    std::string query = queryStream.str();
    if (mysql_query(&mysql, query.c_str()) != 0) {
        return "Error: can't registration, because: " + std::string(mysql_error(&mysql));
    }
    else {
        std::string ok = "Registration success\n";
        return  ok;
    }
}


std::string loginUser(MYSQL& mysql, const std::string& login, const std::string& password)
{
    std::ostringstream queryStream;
    queryStream << "SELECT * FROM testdb.users WHERE name = '" << login << "' AND password = '" << password << "'";
    std::string query = queryStream.str();
    if (mysql_query(&mysql, query.c_str()) != 0) {
        return "Error: can't login, because: " + std::string(mysql_error(&mysql));
    }
    else {
        std::string ok = "login success\n";
        return  ok;
    }
}


int main()
{
    // start server
    io_service service;
    ip::tcp::endpoint endpoint(ip::tcp::v4(), 1234);
    ip::tcp::acceptor acceptor(service, endpoint);
    boost::system::error_code error;
    std::cout << "Server started. Listening on port 1234" << std::endl;
    
    // connect to SQL server
    MYSQL mysql;
    MYSQL_RES* res;
    MYSQL_ROW row;

    // �������� ���������� ����������
    mysql_init(&mysql);
    if (&mysql == nullptr) {
        // ���� ���������� �� ������� � ������� ��������� �� ������
        std::cout << "Error: can't create MySQL-descriptor" << std::endl;
    }

    // ������������ � ��
    if (!mysql_real_connect(&mysql, "localhost", "root", "1234567890", "testdb", NULL, NULL, 0)) {
        // ���� ��� ����������� ���������� ���������� � �� ������� ��������� �� ������
        std::cout << "Error: can't connect to database " << mysql_error(&mysql) << std::endl;
    }


    while (true)
    {
        boost::asio::ip::tcp::socket socket(service);

        acceptor.accept(socket);
        boost::asio::ip::tcp::endpoint clientEndPoint = socket.remote_endpoint();
        std::cout << "Client connected" << std::endl;
        std::cout << clientEndPoint << std::endl;

        // ��������� json �� �������
        std::string jsonData;
        boost::asio::streambuf receiveBuffer;
        boost::asio::read_until(socket, receiveBuffer, "\n", error);
        std::istream is(&receiveBuffer);
        std::getline(is, jsonData);

        // ��������� json
        auto json = boost::json::parse(jsonData);
        std::string api = json.at("api").as_string().c_str();
        std::string login = json.at("login").as_string().c_str();
        std::string password = json.at("password").as_string().c_str();
        std::string recipient = json.at("recipient").as_string().c_str();
        std::string message = json.at("message").as_string().c_str();
        

        // ������ ���-�� � ����������� �� ���� ��� ������� ������

        if (api == "Registration") {
            // ������ ����������� ��� � ��
            std::string reg = registrationUser(mysql, login, password);
            // ���������� ����� �������
            boost::json::object response;
            response["response_message"] = reg;

            std::string jsonDataResponse = boost::json::serialize(response);
            boost::system::error_code error;
            boost::asio::write(socket, buffer(jsonDataResponse + "\n"), error);
            if (error) {
                std::cerr << "Error sending JSON request: " << error.message() << std::endl;
                return 1;
            }
            
        }
        else if (api == "Login") {
            std::string log = loginUser(mysql, login, password);

            // ���������� ����� �������
            boost::json::object response;
            response["response_message"] = log;

            std::string jsonDataResponse = boost::json::serialize(response);
            boost::system::error_code error;
            boost::asio::write(socket, buffer(jsonDataResponse + "\n"), error);
            if (error) {
                std::cerr << "Error sending JSON request: " << error.message() << std::endl;
                return 1;
            }
        }
        else if (api == "Message") {
            /// das
        }
        else {
            // ����������� API
        }
    }

    // ��������� ���������� � ��
    mysql_close(&mysql);
    system("Pause");

    return 0;
}
