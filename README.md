# Qiniu

## 部署：一键 Docker Compose

本项目包含三部分服务：

- server：C++ 后端（HTTP，默认 8080），依赖 MySQL、libcurl、OpenSSL、jsoncpp、mysqlclient；
- py_server：Python FastAPI（默认 8090），调用 DashScope/Qwen；
- web：Vite 构建的前端，Nginx 静态服务（默认 80）。

我们已提供 docker-compose.yml 和对应 Dockerfile，可一键启动完整栈。

### 1. 准备环境变量（必读）

把根目录的 `.env.example` 复制为 `.env` 并按需修改。Docker Compose 会自动加载同目录下的 `.env`：

```powershell
Copy-Item .env.example .env
```

环境变量一览（括号内为默认值）：

- 数据库（由 mysql 容器初始化使用，同时也用于 C++ 服务连接）

  - MYSQL_ROOT_PASSWORD (rootpass)：MySQL root 密码，仅用于容器初始化与健康检查。
  - MYSQL_DATABASE (Tasks)：默认创建的数据库名。
  - MYSQL_USER (Qiniu)：业务用户。
  - MYSQL_PASSWORD (Password)：业务用户密码。
  - MYSQL_HOST (mysql)：C++ 服务访问数据库时的主机名，保持为服务名 `mysql` 即可。
  - MYSQL_PORT (3306)：数据库端口。

- 模型与静态资源

  - MODEL_FS_BASE_DIR (/var/www/models)：容器内模型文件目录（由具名卷 `models-data` 挂载）。
  - MODEL_URL_BASE_PATH (/models)：C++ 服务对外暴露的模型 URL 前缀。

- 腾讯云（可选，用于 C++ 构建期宏覆盖）

  - TENCENTCLOUD_SECRET_ID、TENCENTCLOUD_SECRET_KEY、TENCENTCLOUD_REGION (ap-guangzhou)

- Python 服务（py-server）
  - DASHSCOPE_API_KEY：若使用 DashScope/Qwen 能力需设置。
  - 其他可选：`py-server/.env.example` 中包含 QWEN_MODEL、端口等可调参数，通常保持默认即可。

这些变量的使用位置：

- docker-compose.yml
  - mysql 服务读取 MYSQL\_\* 变量进行初始化（root 密码、库、用户、密码）。
  - server 服务在“构建阶段”通过 Build Args 读取 MYSQL*\*、MODEL*\_、TENCENTCLOUD\_\_，以编译时宏的方式写入可执行程序。
  - py_server（py-server）在“运行阶段”读取 DASHSCOPE_API_KEY 等环境变量。

重要说明：

- 因为 C++ server 的数据库与路径配置是“编译期宏”，如果修改了 MYSQL*\* 或 MODEL*\* 等变量，请重新构建 server 镜像：
  - 仅重建 server：
    ```powershell
    docker compose build server; docker compose up -d server
    ```
  - 或重建全部服务：
    ```powershell
    docker compose build; docker compose up -d
    ```
- Python 服务（py_server）的 DASHSCOPE_API_KEY 属于运行时变量，修改后重启容器即可生效：
  ```powershell
  docker compose up -d py_server
  ```

### 2. 构建并启动

在仓库根目录执行（PowerShell）：

```powershell
docker compose build;

docker compose up -d
```

启动后：

- 前端：http://localhost/
- C++ 服务健康检查：http://localhost:8080/health
- Python 服务根接口：http://localhost:8090/

### 3. 目录与数据持久化

- MySQL 数据：具名卷 mysql-data
- 模型文件：具名卷 models-data，容器内路径默认为 /var/www/models（可通过 .env 修改）

### 4. 常见问题

- 如果构建 server 失败，确认网络可访问依赖镜像源，或切换为国内源。
- 若需要自定义 C++ 连接 MySQL 的编译时宏，请在 .env 中覆盖 MYSQL_HOST、MYSQL_USER 等；Dockerfile 会在构建时写入 include/config.h。

### 5. 快速检查与排错

- 检查环境文件已被加载：
  - 执行 `docker compose config`，在输出中能看到变量已被替换为具体值（注意不要在命令行中泄露敏感信息）。
- 检查 MySQL 容器健康：
  ```powershell
  docker ps --filter name=qiniu-mysql
  ```
- 检查 C++ 服务健康：
  ```powershell
  curl http://localhost:8080/health
  ```
- 检查 Python 服务：
  ```powershell
  curl http://localhost:8090/
  ```
