#include <iostream>
#include <boost/asio.hpp>
#include <boost/json.hpp>

using namespace boost::asio;

int requst(ip::tcp::socket& socket, const std::string& api, const std::string& login = "", const std::string& password = "", const std::string& message = "", const std::string& recipient = "") {
    // Создание JSON-объекта запроса
    boost::json::object request;
    request["api"] = api;
    request["login"] = login;
    request["password"] = password;
    request["recipient"] = recipient;
    request["message"] = message;

    // Преобразование JSON-объекта в строку
    std::string jsonData = boost::json::serialize(request);

    // Отправка JSON-строки на сервер
    boost::system::error_code error;
    boost::asio::write(socket, buffer(jsonData + "\n"), error);
    if (error) {
        std::cerr << "Error sending JSON request: " << error.message() << std::endl;
        return 1;
    }

    std::cout << "JSON request sent successfully" << std::endl;
}


std::string response(ip::tcp::socket& socket)
{
    std::string jsonData;
    boost::system::error_code error;
    boost::asio::streambuf receiveBuffer;
    boost::asio::read_until(socket, receiveBuffer, "\n", error);
    std::istream is(&receiveBuffer);
    std::getline(is, jsonData);

    auto json = boost::json::parse(jsonData);
    std::string responseMessage = json.at("response_message").as_string().c_str();
  
    return responseMessage;
}



std::string apiRegistration = "Registration";
std::string apiLogin = "Login";
std::string apiMessage = "Message";

class Roma
{
public:
    Roma();
    ~Roma();

private:

};


int main() {
    io_service service;
    ip::tcp::endpoint endpoint(ip::address::from_string("127.0.0.1"), 1234);

    ip::tcp::socket socket(service);
    socket.connect(endpoint);


    int choice = 0;
    std::string login;
    std::string password;
    do
    {
        std::cout << "Welcome! Please choose: 1 - SignUp, 2 - Login, 3 - Exit: ";
        std::cin >> choice;

        switch (choice)
        {
        case 1: // registration
            std::cout << "Enter your login" << std::endl;
            std::cin >> login;
            std::cout << "Enter your password" << std::endl;
            std::cin >> password;
            requst(socket, apiRegistration,login, password, "ROMA TEST MSG", "ROMKA100");
            std::cout << response(socket);
            break;
        case 2: // login
            std::cout << "Enter your login" << std::endl;
            std::cin >> login;
            std::cout << "Enter your password" << std::endl;
            std::cin >> password;
            requst(socket, apiLogin, login, password);
            std::cout << response(socket);
            break;
        case 3: // send message
            break;
        default:
            break;
        }

    } while (choice != 3);


    return 0;
}