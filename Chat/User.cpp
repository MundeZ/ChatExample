#include "User.h"

User::User(ip::tcp::socket socket, MYSQL& mysql, std::vector<std::shared_ptr<User>>& session)
    : socket_(std::move(socket)), mysql_(mysql), sessions_(session) {}

User::~User() {
    socket_.close();
    sessions_.erase(std::remove(sessions_.begin(), sessions_.end(), shared_from_this()), sessions_.end());
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
                        std::map<std::string, std::string> responseMap;
                        responseMap["response_message"] = "ERROR: " + std::string(e.what());
                        do_write(responseMap); 
                    }
                }
                else {
                    Logger::instance().log("Received empty data from client");
                    std::map<std::string, std::string> responseMap;
                    responseMap["response_message"] = "ERROR: Empty data received";
                    do_write(responseMap); 
                }
            }
            else {
                Logger::instance().log("Error reading data: " + ec.message());
                std::map<std::string, std::string> responseMap;
                responseMap["response_message"] = "ERROR: " + ec.message();
                do_write(responseMap); 
            }
        });
}

void User::do_write(const std::map<std::string, std::string>& responseMap) {
    auto self(shared_from_this());

    boost::json::object response;
    for (const auto& pair : responseMap) {
        response[pair.first] = pair.second;
    }
    auto jsonResponse = std::make_shared<std::string>(boost::json::serialize(response) + "\n");

    async_write(socket_, buffer(*jsonResponse),
        [this, self, jsonResponse](boost::system::error_code ec, std::size_t /*length*/) {
            if (ec) {
                Logger::instance().log("Error sending data: " + ec.message());
            }
            else {
                Logger::instance().log("Data sent successfully: " + *jsonResponse);
            }
        });
}

std::map<std::string, std::string> User::parseData(const std::string& json_data) {
    std::map<std::string, std::string> json_data_;
    try {
        boost::json::value json = boost::json::parse(json_data);
        if (json.is_object()) {
            boost::json::object obj = json.as_object();
            for (const auto& item : obj) {
                if (item.value().is_string()) {
                    json_data_.emplace(std::string(item.key().data(), item.key().size()),
                        std::string(item.value().as_string().data(), item.value().as_string().size()));
                }
                else {
                    Logger::instance().log("Value for key " + std::string(item.key().data(), item.key().size()) + " is not a string");
                }
            }
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
        registrationUser(client_data.at("login"), client_data.at("password"));
    }
    else if (client_data.at("api") == "Login") {
        loginUser(client_data.at("login"), client_data.at("password"));
    }
    else if (client_data.at("api") == "FindUser") {
        findUser(client_data.at("login"));
    }
    else if (client_data.at("api") == "Message") {
        sendMessage(client_data.at("recipient"), client_data.at("message"));
    }
}

void User::loginUser(const std::string& login, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream queryStream;
    queryStream << "SELECT COUNT(*) FROM testdb.users WHERE name = '" << login << "' AND password = '" << password << "'";
    std::string query = queryStream.str();

    if (mysql_query(&mysql_, query.c_str()) != 0) {
        std::map<std::string, std::string> responseMap;
        responseMap["api"] = "Login";
        responseMap["response_message"] = "ERROR: " + std::string(mysql_error(&mysql_));
        do_write(responseMap);
    }
    else {
        MYSQL_RES* result = mysql_store_result(&mysql_);
        if (result == nullptr) {
            std::map<std::string, std::string> responseMap;
            responseMap["api"] = "Login";
            responseMap["response_message"] = "ERROR: " + std::string(mysql_error(&mysql_));
            do_write(responseMap);
        }
        else {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row != nullptr && std::stoi(row[0]) > 0) {
                std::map<std::string, std::string> responseMap;
                responseMap["api"] = "Login";
                responseMap["response_message"] = "OK";
                setLogin(login);
                do_write(responseMap);
            }
            else {
                std::map<std::string, std::string> responseMap;
                responseMap["api"] = "Login";
                responseMap["response_message"] = "ERROR: Invalid login or password";
                do_write(responseMap);
            }
            mysql_free_result(result);
        }
    }
}

void User::registrationUser(const std::string& login, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string checkQuery = "SELECT COUNT(*) FROM testdb.users WHERE name = '" + login + "'";

    if (mysql_query(&mysql_, checkQuery.c_str()) != 0) {
        std::map<std::string, std::string> responseMap;
        responseMap["api"] = "Registration";
        responseMap["response_message"] = "ERROR: " + std::string(mysql_error(&mysql_));
        do_write(responseMap);
    }

    MYSQL_RES* res = mysql_store_result(&mysql_);
    MYSQL_ROW row = mysql_fetch_row(res);

    if (row && std::stoi(row[0]) > 0) {
        std::map<std::string, std::string> responseMap;
        responseMap["api"] = "Registration";
        responseMap["response_message"] = "ERROR: User already exists";
        do_write(responseMap);
    }

    mysql_free_result(res);

    std::ostringstream queryStream;
    queryStream << "INSERT INTO testdb.users(id, name, password) VALUES (default, '" << login << "', '" << password << "')";
    std::string query = queryStream.str();

    if (mysql_query(&mysql_, query.c_str()) != 0) {
        std::map<std::string, std::string> responseMap;
        responseMap["api"] = "Registration";
        responseMap["response_message"] = "ERROR: " + std::string(mysql_error(&mysql_));
        do_write(responseMap);
    }
    else {
        std::map<std::string, std::string> responseMap;
        responseMap["api"] = "Registration";
        responseMap["response_message"] = "OK";
        setLogin(login);
        do_write(responseMap);
    }
}

void User::findUser(const std::string& login) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream queryStream;
    queryStream << "SELECT COUNT(*) FROM testdb.users WHERE name = '" << login << "'";
    std::string query = queryStream.str();

    if (mysql_query(&mysql_, query.c_str()) != 0) {
        std::map<std::string, std::string> responseMap;
        responseMap["api"] = "FindUser";
        responseMap["response_message"] = "ERROR: " + std::string(mysql_error(&mysql_));
        do_write(responseMap);
    }
    else {
        MYSQL_RES* result = mysql_store_result(&mysql_);
        if (result == nullptr) {
            std::map<std::string, std::string> responseMap;
            responseMap["api"] = "FindUser";
            responseMap["response_message"] = "ERROR: " + std::string(mysql_error(&mysql_));
            do_write(responseMap);
        }
        else {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row != nullptr && std::stoi(row[0]) > 0) {
                std::map<std::string, std::string> responseMap;
                responseMap["api"] = "FindUser";
                responseMap["response_message"] = "OK";
                do_write(responseMap);
            }
            else {
                std::map<std::string, std::string> responseMap;
                responseMap["api"] = "FindUser";
                responseMap["response_message"] = "ERROR: User not found";
                do_write(responseMap);
            }
            mysql_free_result(result);
        }
    }
}

std::shared_ptr<User> User::findRecipient(const std::string& recipient) {
    auto it = std::find_if(sessions_.begin(), sessions_.end(),
        [&recipient](const std::shared_ptr<User>& user) {
            return user->getLogin() == recipient;
        });
    return (it != sessions_.end()) ? *it : nullptr;
}

void User::sendMessage(const std::string& recipient, const std::string& message) {
    std::shared_ptr<User> recipientUser = findRecipient(recipient);

    std::map<std::string, std::string> responseMap;
    if (recipientUser) {
        Logger::instance().log("Recipient found: " + recipient + ", sending message: " + message);
        responseMap["api"] = "Message";
        responseMap["response_message"] = getLogin() + ": " + message;
        recipientUser->do_write(responseMap);

    }
    else {
        responseMap["api"] = "Message";
        responseMap["response_message"] = "ERROR: Recipient not found";
        Logger::instance().log("Recipient not found: " + recipient);
    }
    do_write(responseMap);
}

std::string User::getLogin() const {
    return login_;
}

void User::setLogin(const std::string& login) {
    login_ = login;
}
