#pragma once

#include <string>

// Helpers to read configuration from environment with sane fallbacks.
// Secrets default to empty string to avoid accidental leakage.

// Generic env reader
std::string getEnvOrDefault(const char* name, const std::string& defValue);

// MySQL runtime configuration
std::string rc_mysql_host();
std::string rc_mysql_user();
std::string rc_mysql_password();
std::string rc_mysql_database();
unsigned int rc_mysql_port();

// Tencent Cloud credentials
std::string rc_tc_secret_id();
std::string rc_tc_secret_key();
std::string rc_tc_region();
