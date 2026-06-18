#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <atomic>
#include <thread>
#include <csignal>


namespace {
    std::string read_file(const std::string& path) {
        std::ifstream file(path);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::string first_line(const std::string& path) {
        std::ifstream file(path);
        std::string line;
        std::getline(file, line);
        return line.empty() ? "n/a" : line;
    }

    std::string memory_used_mb() {
        try{
            std::ifstream file("/proc/meminfo");
            if (!file.is_open()) return "n/a";
            std::string key, unit;
            long total = 0, available = 0, value = 0;
            while (file >> key >> value >> unit) {
                if (key == "MemTotal:") total = value;
                if (key == "MemAvailable:") available = value;
            }
            return total > 0 ? std::to_string((total - available) / 1024) + " MB" : "n/a";
        } catch(...){
            return "n/a";
        }
    }

    std::string get_uptime_min() {
        try {
            std::string line = first_line("/proc/uptime");
            if (line == "n/a") return "n/a";
            std::istringstream uptime(line);
            double seconds = 0;
            uptime >> seconds;
            return std::to_string(static_cast<int>(seconds / 60)) + " min";
        } catch (...) {
            return "n/a";
        }
    }
    std::atomic<bool> running{true};
    void signal_handler(int) { running = false; }

}