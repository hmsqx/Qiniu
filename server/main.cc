#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "comm/httplib.h"

#include "include/handlers.h"

namespace fs = std::filesystem;

int main()
{
    httplib::Server server;
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

    // 添加静态文件服务，用于访问下载的模型文件
    // 添加专门的下载接口
    server.Get("/download/(.*)", [](const httplib::Request &req, httplib::Response &res) {
        std::string file_name = req.matches[1];
        std::string full_file_path = "/root/Qiniu/server/model/" + file_name; 
        
        // 安全检查：确保路径在允许的目录内
        if (full_file_path.find("/root/Qiniu/server/model/") != 0) {
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

    server.Get("/model/.*", [](const httplib::Request &req, httplib::Response &res) {
        std::string filePath = "/root/Qiniu/server" + req.path;
        
        // 安全检查：确保路径在允许的目录内
        if (filePath.find("/root/Qiniu/server/model/") != 0) {
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

    std::cout << "启动HTTP服务器，监听端口8080..." << std::endl;
    std::cout << "模型文件服务已启用，可通过 /model/ 路径访问下载的文件" << std::endl;
    server.listen("0.0.0.0", 8080);
    return 0;
}
