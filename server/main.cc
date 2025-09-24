#include <iostream>
#include "comm/httplib.h"

#include "include/handlers.h"

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

    std::cout << "启动HTTP服务器，监听端口8080..." << std::endl;
    server.listen("0.0.0.0", 8080);
    return 0;
}
