#include "dotenv.h"

#include <fstream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <iostream>

static void trim(std::string& s) {
    auto not_space = [](int ch){ return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
}

static void load_env_file(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return;
    std::string line;
    while (std::getline(in, line)) {
        // strip comments
        auto hash = line.find('#');
        if (hash != std::string::npos) line = line.substr(0, hash);
        trim(line);
        if (line.empty()) continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        trim(key); trim(val);
        if (!key.empty()) {
    #if defined(_WIN32)
            _putenv_s(key.c_str(), val.c_str());
    #else
            setenv(key.c_str(), val.c_str(), 1);
    #endif
        }
    }
}

void load_dotenv_if_present() {
    // Try current directory .env
    load_env_file(".env");
    // Try server/.env (when run from repo root)
    load_env_file("server/.env");
}
