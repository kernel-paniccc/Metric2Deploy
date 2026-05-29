#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include <httplib.h>

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
    std::ifstream file("/proc/meminfo");
    std::string key, unit;
    long total = 0, available = 0, value = 0;
    while (file >> key >> value >> unit) {
        if (key == "MemTotal:") total = value;
        if (key == "MemAvailable:") available = value;
    }
    return total > 0 ? std::to_string((total - available) / 1024) + " MB" : "n/a";
}

std::string render_page() {
    std::string html = read_file("templates/index.html");
    const char* host = std::getenv("HOSTNAME");
    const std::string hostname = host ? host : "local";
    std::istringstream uptime(first_line("/proc/uptime"));
    double seconds = 0;
    uptime >> seconds;

    auto replace_all = [&](const std::string& from, const std::string& to) {
        for (size_t pos = 0; (pos = html.find(from, pos)) != std::string::npos; pos += to.size()) {
            html.replace(pos, from.size(), to);
        }
    };

    replace_all("{{title}}", "Metric2Deploy");
    replace_all("{{host}}", hostname);
    replace_all("{{cpu}}", std::to_string(std::thread::hardware_concurrency()));
    replace_all("{{memory}}", memory_used_mb());
    replace_all("{{uptime}}", std::to_string(static_cast<int>(seconds / 60)) + " min");
    return html;
}
}  // namespace

int main() {
    httplib::Server app;

    app.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(render_page(), "text/html; charset=utf-8");
    });

    app.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    std::cout << "listening on :8000\n";
    app.listen("0.0.0.0", 8000);
}
