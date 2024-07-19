#include "Session.h"


Session::Session(ip::tcp::socket socket, std::vector<std::shared_ptr<Session>>& sessions)
    : socket_(std::move(socket)), sessions_(sessions) {}

Session::~Session() {
    if (!socket_.is_open()) {
        socket_.close();
    }
}

void Session::start() {
    sessions_.push_back(shared_from_this());
    std::cout << "Client connected" << std::endl;
    do_read();
}

void Session::do_read() {
    auto self(shared_from_this());
    socket_.async_read_some(buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::string receivedData = std::string(data_, length);
                std::cout << "Received JSON data from client: " << receivedData << std::endl;

                // Парсинг JSON-данных от клиента
                try {
                    auto json = boost::json::parse(receivedData);
                    if (json.is_object()) {
                        // Обработка запроса от клиента
                        std::string api = json.at("api").as_string().c_str();
                        if (api == "Login") {
                            std::string api = json.at("api").as_string().c_str();
                            std::string login = json.at("login").as_string().c_str();
                            std::string password = json.at("password").as_string().c_str();
                            std::string recipient = json.at("recipient").as_string().c_str();
                            std::string message = json.at("message").as_string().c_str();
                            std::map<std::string, std::string> responseMap;

                            responseMap["response_message"] = "OK";

                            // Отправка ответа клиенту
                            do_write(length, responseMap);
                        }
                        else {
                            // Неизвестная операция или другая обработка
                            std::map<std::string, std::string> responseMap;
                            responseMap["response_message"] = "Unknown operation";
                            do_write(length, responseMap);
                        }
                    }
                    else {
                        std::map<std::string, std::string> responseMap;
                        responseMap["response_message"] = "Invalid JSON format";
                        do_write(length, responseMap);
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                    std::map<std::string, std::string> responseMap;
                    responseMap["response_message"] = "Error parsing JSON";
                    do_write(length, responseMap);
                }
            }
            else {
                handle_disconnect();
            }
        });
}

void Session::do_write(std::size_t length, std::map<std::string, std::string> responseMap) {
    auto self(shared_from_this());

    // Преобразование std::map в JSON-подобную строку
    boost::json::object response;
    for (const auto& pair : responseMap) {
        response[pair.first] = pair.second;
    }
    std::string jsonResponse = boost::json::serialize(response) + "\n";

    // Отправка ответа клиенту
    async_write(socket_, buffer(jsonResponse),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (ec) {
                handle_disconnect();
            }
        });
}

void Session::handle_disconnect() {
    auto self(shared_from_this());
    sessions_.erase(std::remove(sessions_.begin(), sessions_.end(), shared_from_this()), sessions_.end());
}
