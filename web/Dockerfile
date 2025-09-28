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
ENV API_UPSTREAM=${API_UPSTREAM} \
	LLM_UPSTREAM=${LLM_UPSTREAM}
COPY --from=build /app/dist /usr/share/nginx/html

## 生成安全的 Nginx 配置：仅替换我们定义的两个变量，避免破坏 $host/$uri 等 Nginx 变量
RUN set -eux; \
		cat > /tmp/default.conf.template <<'EOF'
server {
	listen 80;
	server_name _;
	root /usr/share/nginx/html;
	index index.html;

	location / {
		try_files $uri $uri/ /index.html;
	}

	location /api/ {
		proxy_pass ${API_UPSTREAM}/;
		proxy_set_header Host $host;
		proxy_set_header X-Real-IP $remote_addr;
		proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
	}

	location /llm/ {
		proxy_pass ${LLM_UPSTREAM}/;
		proxy_set_header Host $host;
		proxy_set_header X-Real-IP $remote_addr;
		proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
	}

	# 静态模型文件从 C++ 服务透传
	location /models/ {
		proxy_pass ${API_UPSTREAM}/models/;
	}
}
EOF

# 仅替换我们需要的变量，保留 Nginx 自身变量不被 envsubst 处理
RUN set -eux; \
		envsubst '${API_UPSTREAM} ${LLM_UPSTREAM}' < /tmp/default.conf.template > /etc/nginx/conf.d/default.conf; \
		rm -f /tmp/default.conf.template

EXPOSE 80
