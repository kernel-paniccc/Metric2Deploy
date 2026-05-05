#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

std::string url_decode(const std::string& value) {
    std::string result;
    result.reserve(value.size());

    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '%' && i + 2 < value.size()) {
            const std::string hex = value.substr(i + 1, 2);
            char decoded = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
            result.push_back(decoded);
            i += 2;
        } else if (value[i] == '+') {
            result.push_back(' ');
        } else {
            result.push_back(value[i]);
        }
    }

    return result;
}

std::map<std::string, std::string> parse_query(const std::string& query) {
    std::map<std::string, std::string> result;
    std::stringstream stream(query);
    std::string pair;

    while (std::getline(stream, pair, '&')) {
        const auto pos = pair.find('=');
        if (pos == std::string::npos) {
            result[url_decode(pair)] = "";
            continue;
        }
        result[url_decode(pair.substr(0, pos))] = url_decode(pair.substr(pos + 1));
    }

    return result;
}

std::string json_escape(const std::string& value) {
    std::string result;
    result.reserve(value.size());

    for (char ch : value) {
        switch (ch) {
            case '\\':
                result += "\\\\";
                break;
            case '"':
                result += "\\\"";
                break;
            case '\n':
                result += "\\n";
                break;
            default:
                result.push_back(ch);
                break;
        }
    }

    return result;
}

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("failed to open file: " + path);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string make_response(
    int status,
    const std::string& status_text,
    const std::string& content_type,
    const std::string& body
) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status << ' ' << status_text << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << body;
    return response.str();
}

std::string now_iso8601() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm utc_tm {};
    gmtime_r(&now_time, &utc_tm);

    std::ostringstream out;
    out << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

std::string handle_request(const std::string& request) {
    std::istringstream stream(request);
    std::string method;
    std::string target;
    std::string version;
    stream >> method >> target >> version;

    if (method != "GET") {
        return make_response(405, "Method Not Allowed", "application/json",
                             "{\"detail\":\"only GET is supported\"}");
    }

    std::string path = target;
    std::string query_string;
    const auto query_pos = target.find('?');
    if (query_pos != std::string::npos) {
        path = target.substr(0, query_pos);
        query_string = target.substr(query_pos + 1);
    }
    const auto query = parse_query(query_string);

    if (path == "/") {
        return make_response(200, "OK", "text/html; charset=utf-8", read_file("templates/index.html"));
    }

    if (path == "/health") {
        return make_response(200, "OK", "application/json", "{\"status\":\"ok\"}");
    }

    if (path == "/time") {
        const auto epoch = std::chrono::duration_cast<std::chrono::seconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();
        std::ostringstream body;
        body << "{\"utc_iso\":\"" << now_iso8601() << "\",\"epoch_seconds\":" << epoch << "}";
        return make_response(200, "OK", "application/json", body.str());
    }

    if (path == "/random") {
        int minimum = 0;
        int maximum = 100;

        if (const auto it = query.find("min"); it != query.end() && !it->second.empty()) {
            minimum = std::stoi(it->second);
        }
        if (const auto it = query.find("max"); it != query.end() && !it->second.empty()) {
            maximum = std::stoi(it->second);
        }
        if (minimum >= maximum) {
            return make_response(400, "Bad Request", "application/json",
                                 "{\"detail\":\"min must be less than max\"}");
        }

        static thread_local std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(minimum, maximum);

        std::ostringstream body;
        body << "{\"value\":" << distribution(generator) << "}";
        return make_response(200, "OK", "application/json", body.str());
    }

    if (path == "/echo") {
        const auto it = query.find("message");
        if (it == query.end() || it->second.empty() || it->second.size() > 200) {
            return make_response(400, "Bad Request", "application/json",
                                 "{\"detail\":\"message is required and must be 1..200 chars\"}");
        }

        return make_response(200, "OK", "application/json",
                             "{\"echo\":\"" + json_escape(it->second) + "\"}");
    }

    if (path == "/info") {
        const char* hostname = std::getenv("HOSTNAME");
        const char* env = std::getenv("ENV");
        const char* stage = std::getenv("STAGE");
        const char* node_name = std::getenv("NODE_NAME");
        const char* pwd = std::getenv("PWD");

        std::ostringstream body;
        body << "{\"hostname\":\"" << json_escape(hostname ? hostname : "unknown") << "\",";
        body << "\"working_dir\":\"" << json_escape(pwd ? pwd : "/app") << "\",";
        body << "\"env_example\":{";

        bool first = true;
        const std::pair<const char*, const char*> env_values[] = {
            {"ENV", env},
            {"STAGE", stage},
            {"NODE_NAME", node_name},
            {"PWD", pwd},
        };

        for (const auto& item : env_values) {
            if (!item.second || std::string(item.second).empty()) {
                continue;
            }
            if (!first) {
                body << ",";
            }
            first = false;
            body << "\"" << item.first << "\":\"" << json_escape(item.second) << "\"";
        }

        body << "}}";
        return make_response(200, "OK", "application/json", body.str());
    }

    return make_response(404, "Not Found", "application/json", "{\"detail\":\"not found\"}");
}

}

int main() {
    const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8000);

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 16) < 0) {
        std::perror("listen");
        close(server_fd);
        return 1;
    }

    while (true) {
        sockaddr_in client_address {};
        socklen_t client_len = sizeof(client_address);
        const int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&client_address), &client_len);
        if (client_fd < 0) {
            continue;
        }

        char buffer[8192] = {0};
        const ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            try {
                const std::string response = handle_request(std::string(buffer, bytes_read));
                send(client_fd, response.c_str(), response.size(), 0);
            } catch (const std::exception&) {
                const std::string response = make_response(
                    500, "Internal Server Error", "application/json", "{\"detail\":\"internal server error\"}");
                send(client_fd, response.c_str(), response.size(), 0);
            }
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
