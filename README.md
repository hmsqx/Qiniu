# 项目 Docker 启动与环境变量完整说明

本文面向首次接手与日常运维，提供一键启动、全部环境变量解释、健康检查与排错指引。命令以 Windows PowerShell 为例。

同时我们初始化了部分数据。用来展示功能的效果。

## 目录与服务概览

- `docker-compose.yml`：编排文件，包含 4 个服务与 2 个数据卷
- `.env`：集中管理环境变量（compose 会自动加载）
- `server/`：C++ 服务（对接腾讯云 AI3D 等），端口 8080
- `py-server/`：Python API（文本/图片增强与 Qwen 推理），端口 8090
- `web/`：前端（Nginx 静态与反代），端口 80
- `seed-models/`：初始种子模型目录（只读挂载进 container）

容器与端口映射：

- MySQL：容器 3306（不对外暴露），服务名 `mysql`
  数据库的各种配置在.env 配置就可以了
- Server：容器 8080 → 本机 8080
- Py Server：容器 8090 → 本机 8090
- Web：容器 80 → 本机 80

共享卷：

- `mysql-data` → `/var/lib/mysql`
- `models-data` → `${MODEL_FS_BASE_DIR}`（默认 `/var/www/models`）

## 一键启动

在仓库根目录执行：

```powershell
docker compose build --no-cache
docker compose up -d
```

启动顺序和健康检查：

- `mysql` 健康后，`server` 才会启动
- `server` 启动后，`py_server`、`web` 才会陆续启动

- 健康探针：
  - Server: http://localhost:8080/health
  - Py Server: http://localhost:8090/

访问入口：

- 前端：http://localhost/
- 后端（C++）：http://localhost:8080/
- Python API：http://localhost:8090/

## 全量环境变量与构建参数说明（按服务分组）

说明字段：

- 作用范围：Build（镜像构建时）/ Runtime（容器运行时）
- 默认值：来自 compose 或代码默认
- 敏感：是/否（涉及密钥）

### 1) 全局/.env（compose 会注入到各服务）

- `UBUNTU_MIRROR`（Build，可空，非敏感）
  - 用于 `server` 镜像 apt 源加速，例如 `http://mirrors.aliyun.com/ubuntu`
- `TZ`（Runtime，默认 `Asia/Shanghai`，非敏感）
  - 传给各容器设置时区

### 2) MySQL 服务 mysql

- `MYSQL_ROOT_PASSWORD`（Runtime，无默认，敏感）
  - MySQL root 密码
- `MYSQL_DATABASE`（Runtime，默认 `Tasks`，非敏感）
  - 初始化创建的数据库名
- `MYSQL_USER`（Runtime，默认 `Qiniu`，非敏感）
  - 普通账号名
- `MYSQL_PASSWORD`（Runtime，默认 `Password`，敏感）
  - 普通账号密码

卷/初始化：

- `mysql-data` → `/var/lib/mysql`
- `./server/sql` → `/docker-entrypoint-initdb.d:ro`（启动时执行 .sql/.sh 初始化）

### 3) C++ 后端服务 server

构建参数（Build ARG）：

- `MODEL_FS_BASE_DIR`（默认 `/var/www/models`）
- `MODEL_URL_BASE_PATH`（默认 `/models`）
- `TENCENTCLOUD_REGION`（默认 `ap-guangzhou`）
- `UBUNTU_MIRROR`（见上）

运行时环境变量（Runtime ENV）：

- `TZ`（见全局）
- `MYSQL_HOST`（默认 `mysql`）
- `MYSQL_USER`（默认 `Qiniu`）
- `MYSQL_PASSWORD`（默认空；推荐通过 `.env` 传入，敏感）
- `MYSQL_DATABASE`（默认 `Tasks`）
- `MYSQL_PORT`（默认 `3306`）
- `TENCENTCLOUD_SECRET_ID`（默认为空，敏感）
- `TENCENTCLOUD_SECRET_KEY`（默认为空，敏感）
- `TENCENTCLOUD_REGION`（默认 `ap-guangzhou`）
- 模型种子相关（由 `server/entrypoint.sh` 使用）：
  - `ALWAYS_SEED_MODELS`（默认 `0`）是否每次启动都复制种子模型
  - `SEED_OVERWRITE`（默认 `0`）复制时是否覆盖已存在文件
  - `MODEL_FS_BASE_DIR`（默认 `/var/www/models`）模型目标目录
  - `SEED_MODELS_DIR`（默认 `/opt/seed-models`）种子目录（只读挂载）

卷/端口：

- `models-data` → `${MODEL_FS_BASE_DIR}`（持久化模型）
- `./seed-models` → `${SEED_MODELS_DIR}:ro`（种子）
- 端口：8080（映射至宿主 8080）

健康检查：`GET /health`

### 4) Python 服务 py_server

构建阶段：无额外 ARG，安装 `requirements.txt` 后复制源码。

运行时环境变量（compose → Dockerfile → entry.sh → 应用）：

- `TZ`（见全局）
- `APP_ENV_FILE`（默认 `/app/.env`；项目中 `core/settings.py` 默认使用 `py-server/api.env`，若提供此变量则加载指定路径）
- `DASHSCOPE_API_KEY`（默认空，敏感）用于调用 DashScope/Qwen
- `QWEN_MODEL`（默认 `qwen-plus`）
- `QWEN_IMAGE_EDIT_MODEL`（默认 `qwen-image-edit`）
- `QWEN_COMPATIBLE_BASE_URL`（默认 `https://dashscope.aliyuncs.com/compatible-mode/v1`）
- `DASHSCOPE_BASE_HTTP_API_URL`（默认 `https://dashscope.aliyuncs.com/api/v1`）
- 运行参数（compose 经环境变量传入，docker-entrypoint.sh 解析）：
  - `HOST`（默认 `0.0.0.0`）
  - `PORT`（默认 `8090`）
  - `RELOAD`（默认 `true`）
  - `LOG_LEVEL`（默认 `INFO`）

端口：8090（映射至宿主 8090）

健康检查：`GET /` 返回应用信息

### 5) 前端服务 web（Nginx）

构建参数（Build ARG）：

- `API_UPSTREAM`（默认 `http://server:8080`）后端 API 反代地址
- `LLM_UPSTREAM`（默认 `http://py_server:8090`）LLM API 反代地址

运行时环境变量：无（使用构建期写入的 Nginx 配置）。

端口：80（映射至宿主 80）

## 使用范例与小贴士

### 首次或变更大量依赖后建议全量重建

```powershell
docker compose down -v; docker compose build --no-cache; docker compose up -d
```

### 查看健康状态与日志

```powershell
docker ps
docker logs qiniu-mysql
docker logs qiniu-server
docker logs qiniu-py-server
docker logs qiniu-web
```

### 快速验证

- 打开 http://localhost/
- 打开 http://localhost:8080/health 显示 200 表示后端 C++ 健康
- 打开 http://localhost:8090/ 返回应用信息 JSON 表示 Python 健康

## 常见问题排查（FAQ）

- 端口占用：确保宿主 80/8080/8090 未被占用（Windows 可用 `Get-NetTCPConnection | ? {$_.LocalPort -in 80,8080,8090}` 查看）
- 环境变量未生效：
  - 确认 `.env` 位于仓库根目录且未被重命名
  - 修改 `.env` 后需 `docker compose up -d --build` 使其生效
- MySQL 初始化失败：检查 `server/sql` 中的 SQL 是否语法正确且以 `.sql` 结尾；查看 `qiniu-mysql` 日志
- 模型未注入：检查 `seed-models/` 是否有内容；如需强制复制设置 `ALWAYS_SEED_MODELS=1`，如需覆盖设置 `SEED_OVERWRITE=1`
- 腾讯云未授权：确保 `TENCENTCLOUD_SECRET_ID/KEY` 已设置并传入 `server` 服务
- Py Server 提示缺少密钥：设置 `DASHSCOPE_API_KEY`（可通过 `.env` 或容器环境变量注入）

## 维护与清理

```powershell
# 停止但保留数据
docker compose down

# 停止并清空卷（mysql-data/models-data）
docker compose down -v

# 仅重启某个服务
docker compose restart server

# 进入容器排查
docker exec -it qiniu-server bash
docker exec -it qiniu-py-server sh
```

## 变量清单对照（速查）

- 全局：`UBUNTU_MIRROR`(Build)、`TZ`
- mysql：`MYSQL_ROOT_PASSWORD`(S)、`MYSQL_DATABASE`、`MYSQL_USER`、`MYSQL_PASSWORD`(S)
- server（Build）：`MODEL_FS_BASE_DIR`、`MODEL_URL_BASE_PATH`、`TENCENTCLOUD_REGION`、`UBUNTU_MIRROR`
- server（Runtime）：`MYSQL_HOST`、`MYSQL_USER`、`MYSQL_PASSWORD`(S)、`MYSQL_DATABASE`、`MYSQL_PORT`、`TENCENTCLOUD_SECRET_ID`(S)、`TENCENTCLOUD_SECRET_KEY`(S)、`TENCENTCLOUD_REGION`、`ALWAYS_SEED_MODELS`、`SEED_OVERWRITE`、`MODEL_FS_BASE_DIR`、`SEED_MODELS_DIR`
- py_server：`APP_ENV_FILE`、`DASHSCOPE_API_KEY`(S)、`QWEN_MODEL`、`QWEN_IMAGE_EDIT_MODEL`、`QWEN_COMPATIBLE_BASE_URL`、`DASHSCOPE_BASE_HTTP_API_URL`、`HOST`、`PORT`、`RELOAD`、`LOG_LEVEL`
- web（Build）：`API_UPSTREAM`、`LLM_UPSTREAM`
