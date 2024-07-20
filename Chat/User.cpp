#include "User.h"



User::User(ip::tcp::socket socket, MYSQL& mysql, std::vector<std::shared_ptr<User>>& session)
    : socket_(std::move(socket)), mysql_(mysql) ,sessions_(session) {}

User::~User() {
    socket_.close();
}

void User::start() {
    Logger::instance().log("Client connected: " + socket_.remote_endpoint().address().to_string());
    Logger::instance().log("Waiting for client data...");
    do_read();
}

void User::do_read() {
    auto self(shared_from_this());
    socket_.async_read_some(buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                if (length > 0) {
                    std::string json_data(data_, length);
                    Logger::instance().log("Received JSON data from client: " + json_data);
                    try {
                        std::map<std::string, std::string> client_data = parseData(json_data);
                        menu(client_data);
                        do_read();
                    }
                    catch (const std::exception& e) {
                        Logger::instance().log("Error parsing JSON data: " + std::string(e.what()));
                    }
                }
                else {
                    Logger::instance().log("Received empty data from client");
                }
            }
            else {
                Logger::instance().log("Error reading data: " + ec.message());
            }
        });
}

void User::do_write(std::size_t length, std::map<std::string, std::string> responseMap) {
    auto self(shared_from_this());

    // Преобразование std::map в JSON-подобную строку
    boost::json::object response;
    for (const auto& pair : responseMap) {
        response[pair.first] = pair.second;
    }
    auto jsonResponse = std::make_shared<std::string>(boost::json::serialize(response) + "\n");

    // Отправка ответа клиенту
    async_write(socket_, buffer(*jsonResponse),
        [this, self, jsonResponse](boost::system::error_code ec, std::size_t /*length*/) {
            if (ec) {
                Logger::instance().log("Error sending data: " + ec.message());
            }
        });
}

std::map<std::string, std::string> User::parseData(std::string json_data) {
    std::map<std::string, std::string> json_data_;
    try {
        auto json = boost::json::parse(json_data);
        if (json.is_object()) {
            auto obj = json.as_object();
            json_data_["api"] = obj.at("api").as_string().c_str();
            json_data_["login"] = obj.at("login").as_string().c_str();
            json_data_["password"] = obj.at("password").as_string().c_str();
            json_data_["recipient"] = obj.at("recipient").as_string().c_str();
            json_data_["message"] = obj.at("message").as_string().c_str();
        }
        else {
            Logger::instance().log("Invalid JSON format: not an object");
        }
    }
    catch (const std::exception& e) {
        Logger::instance().log("Exception while parsing JSON: " + std::string(e.what()));
    }
    return json_data_;
}

void User::menu(const std::map<std::string, std::string>& client_data) {
    if (client_data.at("api") == "Registration") {
        registrationUser(mysql_, client_data.at("login"), client_data.at("password"));
    }
    else if (client_data.at("api") == "Login") {
        loginUser(mysql_, client_data.at("login"), client_data.at("password"));
    }
    else if (client_data.at("api") == "FindUser") {
        findUser(mysql_, client_data.at("login"));
    }
    else if (client_data.at("api") == "Message") {
        //insertMessageIntoDB(mysql_, client_data.at("login"), client_data.at("recipient"), client_data.at("message")); // просто для хранения логов 
        //sendMessage(client_data.at("recipient"), client_data.at("message"));
    }
}

void User::loginUser(MYSQL& mysql, const std::string& login, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream queryStream;
    queryStream << "SELECT COUNT(*) FROM testdb.users WHERE name = '" << login << "' AND password = '" << password << "'";
    std::string query = queryStream.str();

    if (mysql_query(&mysql, query.c_str()) != 0) {
        std::map<std::string, std::string> responseMap;
        responseMap["response_message"] = "ERROR: " + std::string(mysql_error(&mysql));
        do_write(max_length, responseMap);
    }
    else {
        MYSQL_RES* result = mysql_store_result(&mysql);
        if (result == nullptr) {
            std::map<std::string, std::string> responseMap;
            responseMap["response_message"] = "ERROR: " + std::string(mysql_error(&mysql));
            do_write(max_length, responseMap);
        }
        else {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row != nullptr && std::stoi(row[0]) > 0) {
                std::map<std::string, std::string> responseMap;
                responseMap["response_message"] = "OK";
                do_write(max_length, responseMap);
            }
            else {
                std::map<std::string, std::string> responseMap;
                responseMap["response_message"] = "ERROR: Invalid login or password";
                do_write(max_length, responseMap);
            }
            mysql_free_result(result);
        }
    }
}

void User::registrationUser(MYSQL& mysql, const std::string& login, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string checkQuery = "SELECT COUNT(*) FROM testdb.users WHERE name = '" + login + "'";

    if (mysql_query(&mysql, checkQuery.c_str()) != 0) {
        std::map<std::string, std::string> responseMap;
        responseMap["response_message"] = "ERROR: " + std::string(mysql_error(&mysql));
        do_write(max_length, responseMap);
    }

    MYSQL_RES* res = mysql_store_result(&mysql);
    MYSQL_ROW row = mysql_fetch_row(res);

    if (row && std::stoi(row[0]) > 0) {
        std::map<std::string, std::string> responseMap;
        responseMap["response_message"] = "ERROR: User already exists";
        do_write(max_length, responseMap);
    }

    mysql_free_result(res);

    std::ostringstream queryStream;
    queryStream << "INSERT INTO testdb.users(id, name, password) VALUES (default, '" << login << "', '" << password << "')";
    std::string query = queryStream.str();

    if (mysql_query(&mysql, query.c_str()) != 0) {
        std::map<std::string, std::string> responseMap;
        responseMap["response_message"] = "ERROR: " + std::string(mysql_error(&mysql));
        do_write(max_length, responseMap);
    }
    else {
        std::map<std::string, std::string> responseMap;
        responseMap["response_message"] = "OK";
        do_write(max_length, responseMap);
    }
}

void User::findUser(MYSQL& mysql, const std::string& login) {

    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream queryStream;
    queryStream << "SELECT COUNT(*) FROM testdb.users WHERE name = '" << login << "'";
    std::string query = queryStream.str();

    if (mysql_query(&mysql, query.c_str()) != 0) {
        std::map<std::string, std::string> responseMap;
        responseMap["response_message"] = "ERROR: " + std::string(mysql_error(&mysql));
        do_write(max_length, responseMap);
    }
    else {
        MYSQL_RES* result = mysql_store_result(&mysql);
        if (result == nullptr) {
            std::map<std::string, std::string> responseMap;
            responseMap["response_message"] = "ERROR: " + std::string(mysql_error(&mysql));
            do_write(max_length, responseMap);
        }
        else {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row != nullptr && std::stoi(row[0]) > 0) {
                std::map<std::string, std::string> responseMap;
                responseMap["response_message"] = "OK";
                do_write(max_length, responseMap);
            }
            else {
                std::map<std::string, std::string> responseMap;
                responseMap["response_message"] = "ERROR: User not found";
                do_write(max_length, responseMap);
            }
            mysql_free_result(result);
        }
    }
}

void User::sendMessage(const std::string& recipient) {
    
}
