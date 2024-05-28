#pragma once

#include <iostream>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/attributes/timer.hpp>
#include <string>
#include <memory>
#include <boost/filesystem.hpp>
#include <mutex>


namespace fs = boost::filesystem;

class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void log(const std::string& message, boost::log::trivial::severity_level severity = boost::log::trivial::info) {
        BOOST_LOG_SEV(lg_, severity) << message;
        std::cout << message << "\n";
        boost::log::core::get()->flush();
    }

private:
    std::shared_mutex mutex_;
    Logger() {
        init();
    }

    ~Logger() {}

    void init() {
        mutex_.lock_shared();
        
        if (!fs::exists("logs")) {
            fs::create_directory("logs");
        }

        boost::log::add_file_log(
            boost::log::keywords::auto_flush = true,
            boost::log::keywords::file_name = "logs/sample_%N.txt",
            boost::log::keywords::rotation_size = 10 * 1024 * 1024,
            boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
            boost::log::keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%]: %Message%"
        );

        boost::log::add_common_attributes();
        mutex_.unlock_shared();
    }

    boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> lg_;
};