#include "User.h"

User::User(ip::tcp::socket socket, MYSQL& mysql) : socket_(std::move(socket)), mysql_(mysql) {}

User::~User() { socket_.close(); }

void User::start() {
	//std::cout << "Client connected: " << socket_.remote_endpoint() << "\n";
    Logger::instance().log("Client connected: " + socket_.remote_endpoint().address().to_string());
    std::map<std::string, std::string> client_data = get_data();   
    std::thread wait_data([this,client_data]() {
        std::cout << "Processing client data...\n";
        menu(client_data);
        });
    wait_data.join();
}

std::map<std::string, std::string> User::get_data() { // get data from client
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
    
    std::map<std::string, std::string> json_data_{ {"api", api}, {"login", login},{"password", password},{"recipient", recipient},{"message", message}, };

    return json_data_;
}

void User::menu(const std::map<std::string, std::string>& client_data) {
    for (const auto& it: client_data) {
        if (it.first == "api" && it.second == "Registration") {
            registrationUser(mysql_,client_data.at("login"), client_data.at("password"));
            
        }
    }
}

void User::registrationUser(MYSQL& mysql, const std::string& login, const std::string& password) {
    mutex_.lock();
    std::ostringstream queryStream;
    queryStream << "INSERT INTO testdb.users(id, name, password) values (default, '" << login << "', '" << password << "')";
    std::string query = queryStream.str();
    mutex_.unlock();
    if (mysql_query(&mysql, query.c_str()) != 0) {
        std::string data = "ERROR";
        Logger::instance().log(data + " " + std::string(mysql_error(&mysql)));
        response(data);
    }
    else {
        std::string data = "OK";
        Logger::instance().log("OK");
        response(data);
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