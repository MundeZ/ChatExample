#include "User.h"

User::User(ip::tcp::socket socket, MYSQL& mysql) : socket_(std::move(socket)), mysql_(mysql) {}

User::~User() { socket_.close(); }

void User::start() {
    try {
        Logger::instance().log("Client connected: " + socket_.remote_endpoint().address().to_string());
        while (true) {
            Logger::instance().log("Waiting for client data...");
            std::map<std::string, std::string> client_data = get_data();
            if (client_data.empty()) {
                Logger::instance().log("No client data received, ending connection.");
                break;
            }
            std::cout << "Processing client data...\n";
            Logger::instance().log("Processing client data...");
            menu(client_data);
        }
    }
    catch (const std::exception& e) {
        Logger::instance().log("Exception: " + std::string(e.what()));
        response("ERROR: Exception occurred");
    }
}

std::map<std::string, std::string> User::get_data() {
    std::string json_data;
    boost::system::error_code error;
    boost::asio::streambuf receiveBuffer;
    boost::asio::read_until(socket_, receiveBuffer, "\n", error);
    std::istream is(&receiveBuffer);
    std::getline(is, json_data);
    auto json = boost::json::parse(json_data);
    std::string api = json.at("api").as_string().c_str();
    std::string login = json.at("login").as_string().c_str();
    std::string password = json.at("password").as_string().c_str();
    std::string recipient = json.at("recipient").as_string().c_str();
    std::string message = json.at("message").as_string().c_str();

    std::map<std::string, std::string> json_data_ = {
        {"api", api},
        {"login", login},
        {"password", password},
        {"recipient", recipient},
        {"message", message}
    };

    return json_data_;
}

void User::menu(const std::map<std::string, std::string>& client_data) {
    for (const auto& it : client_data) {
        if (it.first == "api" && it.second == "Registration") {
            registrationUser(mysql_, client_data.at("login"), client_data.at("password"));
        }
        else if (it.first == "api" && it.second == "Login") {
            loginUser(mysql_, client_data.at("login"), client_data.at("password"));
        }
        else if (it.first == "api" && it.second == "FindUser") {
            findUser(mysql_, client_data.at("login"));
        }
        else if (it.first == "api" && it.second == "Message") {
            // Нужно сделать добавление в базу
        }
        else if (it.first == "api" && it.second == "KAKOITO REQUEST 4TO BI 4ELU DAT SOOBSHENIE") {
            // 
        }
    }
}

void User::registrationUser(MYSQL& mysql, const std::string& login, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);
    

    std::string checkQuery = "SELECT COUNT(*) FROM testdb.users WHERE name = '" + login + "'";

    if (mysql_query(&mysql, checkQuery.c_str()) != 0) {
        std::string data = "ERROR: " + std::string(mysql_error(&mysql));
        Logger::instance().log(data);
        response(data);
        return;
    }

    MYSQL_RES* res = mysql_store_result(&mysql);
    MYSQL_ROW row = mysql_fetch_row(res);

    if (row && std::stoi(row[0]) > 0) {
        std::string data = "ERROR: User already exists";
        Logger::instance().log(data);
        response(data);
        mysql_free_result(res);
        return;
    }

    mysql_free_result(res);

    std::ostringstream queryStream;
    queryStream << "INSERT INTO testdb.users(id, name, password) VALUES (default, '" << login << "', '" << password << "')";
    std::string query = queryStream.str();

    if (mysql_query(&mysql, query.c_str()) != 0) {
        std::string data = "ERROR: " + std::string(mysql_error(&mysql));
        Logger::instance().log(data);
        response(data);
        return;
    }
    else {
        std::string data = "OK";
        Logger::instance().log(data);
        response(data);
    }
}

void User::findUser(MYSQL& mysql, const std::string& login) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream queryStream;
    queryStream << "SELECT COUNT(*) FROM testdb.users WHERE name = '" << login << "'";
    std::string query = queryStream.str();

    if (mysql_query(&mysql, query.c_str()) != 0) {
        std::string data = "ERROR: " + std::string(mysql_error(&mysql));
        Logger::instance().log(data);
        response(data);
    }
    else {
        MYSQL_RES* result = mysql_store_result(&mysql);
        if (result == nullptr) {
            std::string data = "ERROR: " + std::string(mysql_error(&mysql));
            Logger::instance().log(data);
            response(data);
        }
        else {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row != nullptr && std::stoi(row[0]) > 0) {
                std::string data = "OK";
                Logger::instance().log(data);
                response(data);
            }
            else {
                std::string data = "ERROR: User not found";
                Logger::instance().log(data);
                response(data);
            }
            mysql_free_result(result);
        }
    }
}

void User::insertMessageIntoDB(MYSQL& mysql, const std::string& sender, const std::string& recipient, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream queryStream;
    queryStream << "INSERT INTO testdb.messages(id, sender, recipient, message) VALUES (default, '" << sender << "', '" << recipient << "', '" << message << "')";
    std::string query = queryStream.str();

    if (mysql_query(&mysql, query.c_str()) != 0) {
        std::string data = "ERROR: " + std::string(mysql_error(&mysql));
        Logger::instance().log(data);
        response(data);
        return;
    }
    else {
        std::string data = "OK";
        Logger::instance().log(data);
        response(data);
    }
}

void User::loginUser(MYSQL& mysql, const std::string& login, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream queryStream;
    queryStream << "SELECT COUNT(*) FROM testdb.users WHERE name = '" << login << "' AND password = '" << password << "'";
    std::string query = queryStream.str();

    if (mysql_query(&mysql, query.c_str()) != 0) {
        std::string data = "ERROR: " + std::string(mysql_error(&mysql));
        Logger::instance().log(data);
        response(data);
    }
    else {
        MYSQL_RES* result = mysql_store_result(&mysql);
        if (result == nullptr) {
            std::string data = "ERROR: " + std::string(mysql_error(&mysql));
            Logger::instance().log(data);
            response(data);
        }
        else {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row != nullptr && std::stoi(row[0]) > 0) {
                std::string data = "OK";
                Logger::instance().log(data);
                response(data);
            }
            else {
                std::string data = "ERROR: Invalid login or password";
                Logger::instance().log(data);
                response(data);
            }
            mysql_free_result(result);
        }
    }
}

void User::response(const std::string& data) {
    boost::json::object response;
    response["response_message"] = data;

    std::string jsonDataResponse = boost::json::serialize(response);
    boost::system::error_code error;
    boost::asio::write(socket_, buffer(jsonDataResponse + "\n"), error);
    if (error) {
        std::cerr << "Error sending JSON request: " << error.message() << std::endl;
    }
}
