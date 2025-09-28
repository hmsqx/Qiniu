## Web 前端配置说明

### 开发环境（Vite）

1. 复制 `.env.development.example` 为 `.env.development`，按需修改：

   - `VITE_PROXY_TARGET`（C++ server，默认 http://localhost:8080）
   - `VITE_PROXY_TARGET2`（Python app，默认 http://localhost:8090）

2. 启动 Vite 后，前端请求：
   - `GET /api/**` → 代理到 C++ server
   - `GET /models/**` → 代理到 C++ server（用于图片/模型静态访问）
   - `GET /llm/**` → 代理到 Python app

### 生产环境（Docker）

`web/Dockerfile` 支持通过 build args 设置后端地址，默认指向 docker-compose 服务名：

- `API_UPSTREAM`（默认 `http://server:8080`）
- `LLM_UPSTREAM`（默认 `http://app:8090`）

这些值会用于：

- 生成 `.env.production`（给 Vite 构建使用）
- 注入 Nginx 反代配置（`/api/`、`/models/`、`/llm/`）

如需自定义：

```powershell
docker build --build-arg API_UPSTREAM=http://your-server:8080 --build-arg LLM_UPSTREAM=http://your-app:8090 -t qiniu-web ./web
```
