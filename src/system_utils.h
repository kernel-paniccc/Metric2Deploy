#pragma once
#include <atomic>
#include <string>

std::string read_file(const std::string& path);
std::string first_line(const std::string& path);
std::string memory_used_mb();
std::string get_uptime_min();
extern std::atomic<bool> running;
extern long used_mem_bytes;
void signal_handler(int);