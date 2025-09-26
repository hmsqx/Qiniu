#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <thread>
#include <chrono>
#include "comm/httplib.h"

#include "include/handlers.h"

namespace fs = std::filesystem;

int main()
{
    std::cout << "启动AI3D服务器..." << std::endl;
    
    httplib::Server server;
    
    // 添加错误处理器
    server.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        if (res.status == 404) {
            res.set_content("{\"error\":\"接口不存在\"}", "application/json");
        } else if (res.status == 500) {
            res.set_content("{\"error\":\"服务器内部错误\"}", "application/json");
        }
    });
    
    // API路由
    server.Post("/api/get_model", handleGetModel);
    server.Get("/api/query", handleQueryJobsByPage);
    server.Post("/api/register", handleRegister);
    server.Post("/api/login", handleLogin);
    server.Get("/api/auth/me", handleMe);
    server.Post("/api/downloadModel", handleDownloadModel);
    server.Post("/api/like", handleLikeModel);
    server.Get("/api/showModel", handleShowModel);
    server.Get("/health", [](const httplib::Request &, httplib::Response &res)
              { res.set_content("服务正常运行", "text/plain"); });
    server.Post("/api/IncrTokenCount", handleIncrTokenCount);
    server.Post("/api/toggleJobIsPrivate", handleToggleJobIsPrivate);
    server.Get("/api/getTaskFiles", handleGetTaskFiles);
    server.Post("/api/getTaskFiles", handleGetTaskFiles);

    // 静态文件服务
    server.Get("/download/(.*)", [](const httplib::Request &req, httplib::Response &res) {
        std::string file_name = req.matches[1];
        std::string full_file_path = "/root/Qiniu/server/model/" + file_name;

        // 安全检查
        if (full_file_path.find("/root/Qiniu/server/model/") != 0) {
            res.status = 403;
            res.set_content("访问被拒绝", "text/plain");
            return;
        }

        std::ifstream file(full_file_path, std::ios::binary);
        if (file.is_open()) {
            file.seekg(0, std::ios::end);
            std::streamsize file_size = file.tellg();
            file.seekg(0, std::ios::beg);

            char* buffer = new char[file_size];
            file.read(buffer, file_size);

            std::string ext = fs::path(full_file_path).extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            std::string contentType = "application/octet-stream";
            if (ext == ".obj") contentType = "application/octet-octet-stream";
            else if (ext == ".fbx") contentType = "application/octet-stream";
            else if (ext == ".glb") contentType = "model/gltf-binary";
            else if (ext == ".stl") contentType = "application/octet-stream";
            else if (ext == ".usdz") contentType = "model/vnd.usdz+zip";
            else if (ext == ".mp4") contentType = "video/mp4";
            else if (ext == ".jpg" || ext == ".jpeg") contentType = "image/jpeg";
            else if (ext == ".png") contentType = "image/png";

            res.set_content(buffer, file_size, contentType.c_str());
            res.set_header("Content-Disposition", "attachment; filename=\"" + file_name + "\"");

            delete[] buffer;
            file.close();
        } else {
            res.status = 404;
            res.set_content("文件未找到", "text/plain");
        }
    });

    // 模型文件预览
    server.Get("/model/(.*)", [](const httplib::Request &req, httplib::Response &res) {
        std::string file_name = req.matches[1];
        std::string full_file_path = "/root/Qiniu/server/model/" + file_name;

        if (full_file_path.find("/root/Qiniu/server/model/") != 0) {
            res.status = 403;
            res.set_content("访问被拒绝", "text/plain");
            return;
        }

        std::ifstream file(full_file_path, std::ios::binary);
        if (file.is_open()) {
            file.seekg(0, std::ios::end);
            std::streamsize file_size = file.tellg();
            file.seekg(0, std::ios::beg);

            char* buffer = new char[file_size];
            file.read(buffer, file_size);

            std::string ext = fs::path(full_file_path).extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            std::string contentType = "application/octet-stream";
            if (ext == ".obj") contentType = "application/octet-octet-stream";
            else if (ext == ".fbx") contentType = "application/octet-stream";
            else if (ext == ".glb") contentType = "model/gltf-binary";
            else if (ext == ".stl") contentType = "application/octet-stream";
            else if (ext == ".usdz") contentType = "model/vnd.usdz+zip";
            else if (ext == ".mp4") contentType = "video/mp4";
            else if (ext == ".jpg" || ext == ".jpeg") contentType = "image/jpeg";
            else if (ext == ".png") contentType = "image/png";

            res.set_content(buffer, file_size, contentType.c_str());
            res.set_header("Content-Disposition", "inline");

            delete[] buffer;
            file.close();
        } else {
            res.status = 404;
            res.set_content("文件未找到", "text/plain");
        }
    });

    std::cout << "服务器启动成功，监听端口: 8080" << std::endl;
    std::cout << "访问 http://localhost:8080/health 检查服务状态" << std::endl;
    
    if (!server.listen("0.0.0.0", 8080)) {
        std::cerr << "服务器启动失败" << std::endl;
        return -1;
    }
    
    return 0;
}
