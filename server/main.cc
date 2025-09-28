#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <thread>
#include <chrono>
#include "comm/httplib.h"
#include "dotenv.h"

#include "include/handlers.h"
#include "include/security_middleware.h"
#include "include/server_config.h"
#include "concurrent_downloader.h"
#include "config.h"

namespace fs = std::filesystem;

int main()
{
    // 开发模式: 尝试加载 .env （本地运行方便配置）
    load_dotenv_if_present();

    // 初始化服务器组件
    if (!ServerInitializer::initialize(ServerInitializer::getDefaultConfig())) {
        std::cerr << "服务器初始化失败" << std::endl;
        return -1;
    }
    // 初始化并发下载器（可根据配置调整并发数）
    initializeConcurrentDownloader(4);

    httplib::Server server;

    // 添加错误处理器
    server.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        // 获取客户端IP进行安全检查
        std::string clientIP = getAPISecurityManager().getClientIP(
            req.get_header_value("X-Forwarded-For"),
            req.get_header_value("X-Real-IP")
        );

        // 记录错误请求
        getAPISecurityManager().recordRequest(clientIP, req.path, false);

        // 仅当业务侧未设置响应体时，才填充默认错误JSON，避免覆盖详细错误信息
        if (res.body.empty()) {
            if (res.status == 404) {
                res.set_content("{\"error\":\"接口不存在\"}", "application/json");
            } else if (res.status == 500) {
                res.set_content("{\"error\":\"服务器内部错误\"}", "application/json");
            }
        }
    });

    // 使用安全中间件包装所有API端点
    SECURE_POST(server, "/api/get_model", handleGetModel);
    SECURE_GET(server, "/api/query", handleQueryJobsByPageAsync);
    SECURE_POST(server, "/api/register", handleRegister);
    SECURE_POST(server, "/api/login", handleLogin);
    SECURE_POST(server, "/api/logout", handleLogout);
    SECURE_GET(server, "/api/auth/me", handleMe);
    SECURE_POST(server, "/api/downloadModel", handleDownloadModel);
    SECURE_POST(server, "/api/like", handleLikeModel);
    SECURE_GET(server, "/api/showModel", handleShowModel);
    SECURE_POST(server, "/api/IncrTokenCount", handleIncrTokenCount);
    SECURE_POST(server, "/api/toggleJobIsPrivate", handleToggleJobIsPrivate);
    SECURE_GET(server, "/api/getTaskFiles", handleGetTaskFiles);
    SECURE_POST(server, "/api/getTaskFiles", handleGetTaskFiles);
    SECURE_POST(server, "/api/like/get", handleGetUserLike);
    SECURE_POST(server, "/api/like/toggle", handleToggleUserLike);
    SECURE_POST(server, "/api/view", handleIncrementViewAndGetRates);
    //SECURE_POST(server, "/api/likeRate", handleGetLikeRate);
    //SECURE_POST(server, "/api/downloadRate", handleGetDownloadRate);
    //SECURE_GET(server, "/api/userGrowth", handleGetUserGrowth);
    // 管理员端
    SECURE_GET(server, "/api/admin/overview", handleAdminOverview);
    SECURE_GET(server, "/api/admin/users", handleAdminQueryUsers);
    SECURE_GET(server, "/api/admin/models", handleAdminQueryModels);

    // 健康检查端点（不需要安全中间件）
    server.Get("/health", [](const httplib::Request &, httplib::Response &res)
              { res.set_content("服务正常运行", "text/plain"); });

    // 添加静态文件服务（如由Nginx托管，此段可作为后备）
    // 添加专门的下载接口
    server.Get("/download/(.*)", [](const httplib::Request &req, httplib::Response &res) {
        std::string file_name = req.matches[1];
        std::string full_file_path = std::string(MODEL_FS_BASE_DIR) + "/" + file_name;

        // 安全检查：确保路径在允许的目录内
        if (full_file_path.find(std::string(MODEL_FS_BASE_DIR) + "/") != 0) {
            res.status = 403;
            res.set_content("访问被拒绝", "text/plain");
            return;
        }

        std::ifstream file(full_file_path, std::ios::binary);
        if (file.is_open()) {
            // 获取文件大小
            file.seekg(0, std::ios::end);
            std::streamsize file_size = file.tellg();
            file.seekg(0, std::ios::beg);

            // 分配缓冲区并读取文件内容到缓冲区
            char* buffer = new char[file_size];
            file.read(buffer, file_size);

            // 根据文件扩展名设置正确的MIME类型
            std::string ext = fs::path(full_file_path).extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            std::string contentType = "application/octet-stream"; // 默认类型
            if (ext == ".obj") contentType = "application/octet-stream";
            else if (ext == ".fbx") contentType = "application/octet-stream";
            else if (ext == ".glb") contentType = "model/gltf-binary";
            else if (ext == ".stl") contentType = "application/octet-stream";
            else if (ext == ".usdz") contentType = "model/vnd.usdz+zip";
            else if (ext == ".mp4") contentType = "video/mp4";
            else if (ext == ".jpg" || ext == ".jpeg") contentType = "image/jpeg";
            else if (ext == ".png") contentType = "image/png";

            // 设置响应内容为文件内容
            res.set_content(buffer, file_size, contentType.c_str());

            // 设置响应头中的Content-Disposition字段，用于告知客户端文件名
            res.set_header("Content-Disposition", "attachment; filename=\"" + file_name + "\"");

            delete[] buffer;
            file.close();
        } else {
            res.status = 404;
            res.set_content("文件未找到", "text/plain");
        }
    });

    server.Get((std::string(MODEL_URL_BASE_PATH) + "/.*").c_str(), [](const httplib::Request &req, httplib::Response &res) {
        std::string filePath = std::string(MODEL_FS_BASE_DIR) + req.path.substr(std::string(MODEL_URL_BASE_PATH).size());

        // 安全检查：确保路径在允许的目录内
        if (filePath.find(std::string(MODEL_FS_BASE_DIR) + "/") != 0) {
            res.status = 403;
            res.set_content("访问被拒绝", "text/plain");
            return;
        }

        // 检查文件是否存在
        if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
            // 根据文件扩展名设置正确的MIME类型
            std::string ext = fs::path(filePath).extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            std::string contentType = "application/octet-stream"; // 默认类型
            if (ext == ".obj") contentType = "application/octet-stream";
            else if (ext == ".fbx") contentType = "application/octet-stream";
            else if (ext == ".glb") contentType = "model/gltf-binary";
            else if (ext == ".stl") contentType = "application/octet-stream";
            else if (ext == ".usdz") contentType = "model/vnd.usdz+zip";
            else if (ext == ".mp4") contentType = "video/mp4";
            else if (ext == ".jpg" || ext == ".jpeg") contentType = "image/jpeg";
            else if (ext == ".png") contentType = "image/png";

            res.set_header("Content-Type", contentType);
            res.set_header("Cache-Control", "public, max-age=3600"); // 缓存1小时

            // 读取并返回文件内容
            std::ifstream file(filePath, std::ios::binary);
            if (file.is_open()) {
                // 获取文件大小
                file.seekg(0, std::ios::end);
                std::streamsize file_size = file.tellg();
                file.seekg(0, std::ios::beg);

                // 分配缓冲区并读取文件内容到缓冲区
                char* buffer = new char[file_size];
                file.read(buffer, file_size);

                // 设置响应内容为文件内容
                res.set_content(buffer, file_size, contentType.c_str());

                // 设置响应头中的Content-Disposition字段，用于告知客户端文件名
                std::string fileName = fs::path(filePath).filename().string();
                res.set_header("Content-Disposition", "inline; filename=\"" + fileName + "\"");

                delete[] buffer;
                file.close();
            } else {
                res.status = 500;
                res.set_content("文件读取失败", "text/plain");
            }
        } else {
            res.status = 404;
            res.set_content("文件未找到", "text/plain");
        }
    });

     std::cout << "启动HTTP服务器，监听端口" << SERVER_PORT << "..." << std::endl;
     std::cout << "模型文件服务已启用，可通过 /model/ 路径访问下载的文件" << std::endl;
     server.listen(SERVER_HOST, SERVER_PORT);
     return 0;
}
