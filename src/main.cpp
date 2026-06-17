#include "system_utils.h"

#include <httplib.h>




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
    replace_all("{{uptime}}", get_uptime_min());
}


int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    httplib::Server app;

    app.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(render_page(), "text/html; charset=utf-8");
    });

    app.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    app.Get("/metrics", [](const httplib::Request&, httplib::Response& res) {
        std::ostringstream out;
        out << "# HELP metric2deploy_uptime_seconds Uptime in seconds\n";
        out << "# TYPE metric2deploy_uptime_seconds gauge\n";
        std::string uptime_line = first_line("/proc/uptime");
        double secs = 0;
        try { secs = std::stod(uptime_line); } catch(...) {}
        out << "metric2deploy_uptime_seconds " << secs << "\n";

        out << "# HELP metric2deploy_memory_used_bytes Memory used\n";
        out << "# TYPE metric2deploy_memory_used_bytes gauge\n";
        out << "metric2deploy_memory_used_bytes " << used_mem_bytes << "\n";

        res.set_content(out.str(), "text/plain; version=0.0.4");
    });

    std::thread server_thread([&]() {
        app.listen("0.0.0.0", 8000);
    });

    std::cout << "listening on :8000\n";
    while (running) std::this_thread::sleep_for(std::chrono::milliseconds(100));

    app.stop();
    server_thread.join();
    std::cout << "Server stopped cleanly.\n";
    return 0;
    
    
}
