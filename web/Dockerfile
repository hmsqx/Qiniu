# 构建前端资源
FROM node:20-alpine AS build
WORKDIR /app

# 构建参数，用于将后端 URL 写入 .env.production
ARG API_UPSTREAM=http://server:8080
ARG LLM_UPSTREAM=http://py_server:8090

COPY package.json pnpm-lock.yaml* ./
RUN corepack enable && corepack prepare pnpm@9.12.3 --activate
RUN pnpm install --frozen-lockfile

COPY . .

RUN printf "VITE_API_BASE=/api\nVITE_PROXY_TARGET=%s\nVITE_ASSET_BASE=/root/Qiniu\nVITE_API_BASE2=/llm\nVITE_PROXY_TARGET2=%s\n" "$API_UPSTREAM" "$LLM_UPSTREAM" > .env.production

RUN pnpm -s build

FROM nginx:1.27-alpine
ARG API_UPSTREAM=http://server:8080
ARG LLM_UPSTREAM=http://py_server:8090
COPY --from=build /app/dist /usr/share/nginx/html

# Nginx 配置，反向代理到后端服务
RUN printf "server {\n  listen 80;\n  server_name _;\n  root /usr/share/nginx/html;\n  index index.html;\n\n  location / {\n    try_files $$uri $$uri/ /index.html;\n  }\n\n  location /api/ {\n    proxy_pass %s/;\n    proxy_set_header Host $$host;\n    proxy_set_header X-Real-IP $$remote_addr;\n    proxy_set_header X-Forwarded-For $$proxy_add_x_forwarded_for;\n  }\n  location /llm/ {\n    proxy_pass %s/;\n    proxy_set_header Host $$host;\n    proxy_set_header X-Real-IP $$remote_addr;\n    proxy_set_header X-Forwarded-For $$proxy_add_x_forwarded_for;\n  }\n  location /models/ {\n    proxy_pass %s/models/;\n  }\n}\n" "$API_UPSTREAM" "$LLM_UPSTREAM" "$API_UPSTREAM" > /etc/nginx/conf.d/default.conf

EXPOSE 80
