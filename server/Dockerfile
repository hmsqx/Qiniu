# C++ 服务器的构建阶段
FROM ubuntu:22.04 AS build

ARG DEBIAN_FRONTEND=noninteractive
ARG UBUNTU_MIRROR

# 构建参数（仅限非敏感信息）。敏感信息通过运行时环境变量或文件提供。
ARG MODEL_FS_BASE_DIR=/var/www/models
ARG MODEL_URL_BASE_PATH=/models
ARG TENCENTCLOUD_REGION=ap-guangzhou

RUN set -eux; \
  # 可选：切换到更快/更近的 Ubuntu 镜像源（如 https://mirrors.aliyun.com/ubuntu）
  if [ -n "${UBUNTU_MIRROR:-}" ]; then \
    sed -i "s|http://archive.ubuntu.com/ubuntu|${UBUNTU_MIRROR}|g; s|http://security.ubuntu.com/ubuntu|${UBUNTU_MIRROR}|g" /etc/apt/sources.list; \
  fi; \
  apt-get update; \
  # 稳定安装，重试三次，避免使用 'gcc' 元包以减少镜像不稳定
  for i in 1 2 3; do \
    apt-get -o Acquire::Retries=3 install -y --no-install-recommends \
    gcc-11 g++-11 make cmake git curl ca-certificates \
    libmysqlclient-dev libcurl4-openssl-dev libssl-dev libjsoncpp-dev \
    && break || (echo "apt install failed ($i/3), retrying..."; sleep $((i*5)); apt-get update --fix-missing); \
  done; \
  rm -rf /var/lib/apt/lists/*

WORKDIR /app/server

# 仅从仓库根目录复制服务器源码
COPY server/ /app/server/

# 构建时不嵌入敏感信息。仅将非敏感路径/区域作为编译常量（如有需要）。
RUN mkdir -p build && cd build \
  && CXXFLAGS="\
    -DMODEL_FS_BASE_DIR=\\\"${MODEL_FS_BASE_DIR}\\\" \
    -DMODEL_URL_BASE_PATH=\\\"${MODEL_URL_BASE_PATH}\\\" \
    -DTENCENTCLOUD_REGION=\\\"${TENCENTCLOUD_REGION}\\\" \
  " \
  cmake -DBUILD_MODULES="ai3d" -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_C_COMPILER=gcc-11 -DCMAKE_CXX_COMPILER=g++-11 .. \
  && make -j$(nproc) \
  && echo "已构建 SDK 库:" \
  && ls -al /app/server/build/lib || true

# 运行时镜像
FROM ubuntu:22.04 AS runtime

ARG DEBIAN_FRONTEND=noninteractive
ARG UBUNTU_MIRROR
RUN set -eux; \
   if [ -n "${UBUNTU_MIRROR:-}" ]; then \
   sed -i "s|http://archive.ubuntu.com/ubuntu|${UBUNTU_MIRROR}|g; s|http://security.ubuntu.com/ubuntu|${UBUNTU_MIRROR}|g" /etc/apt/sources.list; \
   fi; \
   apt-get update; \
   for i in 1 2 3; do \
   apt-get -o Acquire::Retries=3 install -y --no-install-recommends \
     ca-certificates curl libssl3 libcurl4 libnghttp2-14 libjsoncpp25 \
   && break || (echo "apt install failed ($i/3), retrying..."; sleep $((i*5)); apt-get update --fix-missing); \
   done; \
   # MySQL 客户端库名称因仓库不同而异，优先尝试首选项，失败则回退
   (apt-get install -y --no-install-recommends libmysqlclient21 || apt-get install -y --no-install-recommends default-libmysqlclient-dev); \
   rm -rf /var/lib/apt/lists/*

WORKDIR /app

# 模型存储目录
ARG MODEL_FS_BASE_DIR=/var/www/models
RUN mkdir -p ${MODEL_FS_BASE_DIR}
VOLUME ["${MODEL_FS_BASE_DIR}"]

COPY --from=build /app/server/build/tencent_cloud_cpp_sample /usr/local/bin/server

# 复制运行时所需的腾讯云 SDK 动态库
# 这些库通过 BUILD_SHARED_LIBS=ON 构建生成（默认）
# CMake 会将 SDK 库放在顶层 build lib 目录下
COPY --from=build /app/server/build/lib/libtencentcloud-sdk-cpp-core.so* /usr/local/lib/
COPY --from=build /app/server/build/lib/libtencentcloud-sdk-cpp-ai3d.so* /usr/local/lib/

# 同时从构建阶段复制 SDK 库以确保 ABI 匹配

# 确保动态链接器能找到这些库
RUN echo "/usr/local/lib" > /etc/ld.so.conf.d/tencentcloud-sdk.conf && ldconfig \
  && ldconfig -p | grep -E "libcurl\\.so\\.4|libjsoncpp|tencentcloud" || true

# 预检：构建时验证动态依赖是否可解析（如缺失则失败）
RUN echo "server 的 ldd 结果:" && ldd /usr/local/bin/server | tee /tmp/ldd.txt \
  && if grep -q "not found" /tmp/ldd.txt; then echo "检测到未解析的动态库" && cat /tmp/ldd.txt && exit 1; fi

EXPOSE 8080

HEALTHCHECK --interval=10s --timeout=5s --retries=10 CMD curl -f http://localhost:8080/health || exit 1

# 复制入口脚本和默认种子模型目录（默认为空，可通过挂载覆盖）
COPY server/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh && mkdir -p /opt/seed-models

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["/usr/local/bin/server"]
