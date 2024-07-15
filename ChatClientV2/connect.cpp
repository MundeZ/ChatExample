#include "connect.h"

Connect::Connect()
    : socket_(service_)
{}

Connect::~Connect() {
    if (socket_.is_open()) {
        socket_.close();
    }
}

void Connect::connect() {
    ip::tcp::endpoint endpoint(ip::address::from_string("127.0.0.1"), 1234);
    socket_.connect(endpoint, error_);

    if (error_) {
        std::cerr << "Error: " << error_.message() << std::endl;
    } else {
        std::cout << "Successfully connected to server." << std::endl;
    }
}

ip::tcp::socket& Connect::getSocket() {
    return socket_;
}

std::string Connect::getApi(Api api) {
    std::map<Api, std::string> apiToString = {
        { REGISTRATION, "Registration" },
        { LOGIN, "Login" },
        { MESSAGE, "Message" }
    };
    return apiToString[api];
}

std::string Connect::requestToServer(const std::string &api, const std::string &login, const std::string &password, const std::string &message, const std::string &recipient) {
    // Создание JSON-объекта запроса
    boost::json::object request;
    request["api"] = api;
    request["login"] = login;
    request["password"] = password;
    request["recipient"] = recipient;
    request["message"] = message;

    // Преобразование JSON-объекта в строку
    std::string jsonData = boost::json::serialize(request);

    std::string msgResponse;
    // Отправка JSON-строки на сервер
    boost::asio::write(socket_, buffer(jsonData + "\n"), error_);
    if (error_) {
         msgResponse = "Error sending JSON request: " + error_.message();
        return msgResponse;
    }
    return responseFromServer();
}

std::string Connect::responseFromServer() {
        std::string jsonData;
        boost::asio::streambuf receiveBuffer;
        boost::asio::read_until(socket_, receiveBuffer, "\n", error_);
        if (error_) {
            return "Error reading response: " + error_.message();
        }

        std::istream is(&receiveBuffer);
        std::getline(is, jsonData);

        try {
            auto json = boost::json::parse(jsonData);
            if (json.is_object()) {
                auto obj = json.as_object();
                if (obj.contains("response_message")) {
                    return obj["response_message"].as_string().c_str();
                } else {
                    return "Invalid response: missing 'response_message'";
                }
            } else {
                return "Invalid response format";
            }
        } catch (const std::exception& e) {
            return "Error parsing response: " + std::string(e.what());
        }
}


