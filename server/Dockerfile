# Build stage for C++ server
FROM ubuntu:22.04 AS build

ARG DEBIAN_FRONTEND=noninteractive
ARG UBUNTU_MIRROR

# Build args (non-secret only). Secrets are provided at runtime via env or files.
ARG MODEL_FS_BASE_DIR=/var/www/models
ARG MODEL_URL_BASE_PATH=/models
ARG TENCENTCLOUD_REGION=ap-guangzhou

RUN set -eux; \
    # Optionally switch to a faster/closer Ubuntu mirror (e.g. https://mirrors.aliyun.com/ubuntu)
    if [ -n "${UBUNTU_MIRROR:-}" ]; then \
      sed -i "s|http://archive.ubuntu.com/ubuntu|${UBUNTU_MIRROR}|g; s|http://security.ubuntu.com/ubuntu|${UBUNTU_MIRROR}|g" /etc/apt/sources.list; \
    fi; \
    apt-get update; \
    # Robust install with retries and without the 'gcc' meta package to avoid flaky mirrors
    for i in 1 2 3; do \
      apt-get -o Acquire::Retries=3 install -y --no-install-recommends \
        gcc-11 g++-11 make cmake git curl ca-certificates \
        libmysqlclient-dev libcurl4-openssl-dev libssl-dev libjsoncpp-dev \
      && break || (echo "apt install failed ($i/3), retrying..."; sleep $((i*5)); apt-get update --fix-missing); \
    done; \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app/server

# Copy only the server sources from repo root context
COPY server/ /app/server/

# Build without embedding secrets. Only non-secret paths/region as compile-time constants if needed.
RUN mkdir -p build && cd build \
  && CXXFLAGS="\
      -DMODEL_FS_BASE_DIR=\\\"${MODEL_FS_BASE_DIR}\\\" \
      -DMODEL_URL_BASE_PATH=\\\"${MODEL_URL_BASE_PATH}\\\" \
      -DTENCENTCLOUD_REGION=\\\"${TENCENTCLOUD_REGION}\\\" \
    " \
    cmake -DBUILD_MODULES="ai3d" -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_C_COMPILER=gcc-11 -DCMAKE_CXX_COMPILER=g++-11 .. \
  && make -j$(nproc) \
  && echo "Built SDK libraries:" \
  && ls -al /app/server/build/lib || true

# Runtime image
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
   # MySQL client lib name varies per repo; try preferred first, fallback next
   (apt-get install -y --no-install-recommends libmysqlclient21 || apt-get install -y --no-install-recommends default-libmysqlclient-dev); \
   rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Model storage directory
ARG MODEL_FS_BASE_DIR=/var/www/models
RUN mkdir -p ${MODEL_FS_BASE_DIR}
VOLUME ["${MODEL_FS_BASE_DIR}"]

COPY --from=build /app/server/build/tencent_cloud_cpp_sample /usr/local/bin/server

# Copy Tencent Cloud SDK shared libs needed at runtime
# These are produced by building the SDK with BUILD_SHARED_LIBS=ON (default)
# CMake places SDK libraries under the top-level build lib directory
COPY --from=build /app/server/build/lib/libtencentcloud-sdk-cpp-core.so* /usr/local/lib/
COPY --from=build /app/server/build/lib/libtencentcloud-sdk-cpp-ai3d.so* /usr/local/lib/

# Also copy SDK libs from build stage to ensure ABI match

# Ensure the dynamic linker can find the libraries
RUN echo "/usr/local/lib" > /etc/ld.so.conf.d/tencentcloud-sdk.conf && ldconfig \
  && ldconfig -p | grep -E "libcurl\\.so\\.4|libjsoncpp|tencentcloud" || true

# Preflight: verify dynamic dependencies are resolvable at build time (fail if missing)
RUN echo "ldd of server:" && ldd /usr/local/bin/server | tee /tmp/ldd.txt \
  && if grep -q "not found" /tmp/ldd.txt; then echo "Unresolved dynamic libs detected" && cat /tmp/ldd.txt && exit 1; fi

EXPOSE 8080

HEALTHCHECK --interval=10s --timeout=5s --retries=10 CMD curl -f http://localhost:8080/health || exit 1

# Copy entrypoint and default seed directory (empty by default; can be overridden by a bind mount)
COPY server/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh && mkdir -p /opt/seed-models


ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["/usr/local/bin/server"]
