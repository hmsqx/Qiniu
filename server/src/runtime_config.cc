#include "runtime_config.h"
#include "config.h" // for non-secret default constants like MODEL_* if needed

#include <cstdlib>
#include <iostream>

static std::string readEnv(const char* name) {
    const char* v = std::getenv(name);
    return v ? std::string(v) : std::string();
}

std::string getEnvOrDefault(const char* name, const std::string& defValue) {
    auto v = readEnv(name);
    return v.empty() ? defValue : v;
}

// MySQL
std::string rc_mysql_host() {
    return getEnvOrDefault("MYSQL_HOST", "mysql");
}

std::string rc_mysql_user() {
    return getEnvOrDefault("MYSQL_USER", "Qiniu");
}

std::string rc_mysql_password() {
    auto v = readEnv("MYSQL_PASSWORD");
    if (v.empty()) {
        // Development fallback to match compose MySQL default.
        std::cerr << "[WARN] MYSQL_PASSWORD not set; using development default. Configure env in production." << std::endl;
        return std::string("Password");
    }
    return v;
}

std::string rc_mysql_database() {
    return getEnvOrDefault("MYSQL_DATABASE", "Tasks");
}

unsigned int rc_mysql_port() {
    auto v = readEnv("MYSQL_PORT");
    if (v.empty()) return 3306;
    try {
        return static_cast<unsigned int>(std::stoul(v));
    } catch (...) {
        return 3306;
    }
}

// Tencent Cloud
std::string rc_tc_secret_id() {
    return readEnv("TENCENTCLOUD_SECRET_ID");
}

std::string rc_tc_secret_key() {
    return readEnv("TENCENTCLOUD_SECRET_KEY");
}

std::string rc_tc_region() {
    return getEnvOrDefault("TENCENTCLOUD_REGION", "ap-guangzhou");
}
