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
    ip::tcp::endpoint endpoint(ip::address::from_string("127.0.0.1"), 8888);
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
        { MESSAGE, "Message" },
        { FIND_USER, "FindUser" }
    };
    return apiToString[api];
}

std::map<std::string, std::string> Connect::requestToServerRegistration(const std::string &api, const std::string &login, const std::string &password) {
    // Создание JSON-объекта запроса
    boost::json::object request;
    request["api"] = api;
    request["login"] = login;
    request["password"] = password;

    // Преобразование JSON-объекта в строку
    std::string jsonData = boost::json::serialize(request);

    std::string msgResponse;
    // Отправка JSON-строки на сервер
    boost::asio::write(socket_, buffer(jsonData + "\n"), error_);

    return responseFromServer();
}

std::map<std::string, std::string> Connect::requestToServerLogin(const std::string &api, const std::string &login, const std::string &password) {
    // Создание JSON-объекта запроса
    boost::json::object request;
    request["api"] = api;
    request["login"] = login;
    request["password"] = password;

    // Преобразование JSON-объекта в строку
    std::string jsonData = boost::json::serialize(request);

    std::string msgResponse;
    // Отправка JSON-строки на сервер
    boost::asio::write(socket_, buffer(jsonData + "\n"), error_);

    return responseFromServer();
}

std::map<std::string, std::string> Connect::requestToServerMessage(const std::string &api, const std::string &message, const std::string &recipient){
    // Создание JSON-объекта запроса
    boost::json::object request;
    request["api"] = api;
    request["recipient"] = recipient;
    request["message"] = message;

    // Преобразование JSON-объекта в строку
    std::string jsonData = boost::json::serialize(request);

    std::string msgResponse;
    // Отправка JSON-строки на сервер
    boost::asio::write(socket_, buffer(jsonData + "\n"), error_);

    return responseFromServer();
}

std::map<std::string, std::string> Connect::requestToServerFindUser(const std::string &api, const std::string &login) {
    // Создание JSON-объекта запроса
    boost::json::object request;
    request["api"] = api;
    request["login"] = login;

    // Преобразование JSON-объекта в строку
    std::string jsonData = boost::json::serialize(request);

    std::string msgResponse;
    // Отправка JSON-строки на сервер
    boost::asio::write(socket_, buffer(jsonData + "\n"), error_);

    return responseFromServer();
}

std::map<std::string, std::string> Connect::responseFromServer() {
    std::string jsonData;
    std::map<std::string, std::string> json_data_;
    boost::asio::streambuf receiveBuffer;
    boost::system::error_code error;
    boost::asio::read_until(socket_, receiveBuffer, "\n", error);

    std::istream is(&receiveBuffer);
    std::getline(is, jsonData);

    try {
        auto json = boost::json::parse(jsonData);
        if (json.is_object()) {
            auto obj = json.as_object();
            for (const auto& item : obj) {
                if (item.value().is_string()) {
                    json_data_.emplace(std::string(item.key().data(), item.key().size()),
                                       std::string(item.value().as_string().data(), item.value().as_string().size()));
                }
            }
        }
    } catch (const std::exception& e) {
        json_data_["response_message"] = "Error parsing response: " + std::string(e.what());
    }

    return json_data_;
}
