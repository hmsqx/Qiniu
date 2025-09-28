#!/usr/bin/env sh
set -e

# Inject DASHSCOPE_API_KEY into api.env if provided via env
if [ -n "$DASHSCOPE_API_KEY" ]; then
  if [ -f ./api.env ]; then
    # update existing key
    if grep -q '^DASHSCOPE_API_KEY=' ./api.env; then
      sed -i "s#^DASHSCOPE_API_KEY=.*#DASHSCOPE_API_KEY=${DASHSCOPE_API_KEY}#" ./api.env
    else
      echo "DASHSCOPE_API_KEY=${DASHSCOPE_API_KEY}" >> ./api.env
    fi
  else
    echo "DASHSCOPE_API_KEY=${DASHSCOPE_API_KEY}" > ./api.env
  fi
fi

# Server runtime options from env (with defaults)
HOST_ENV=${HOST:-0.0.0.0}
PORT_ENV=${PORT:-8090}
RELOAD_ENV=$(printf "%s" "${RELOAD:-false}" | tr '[:upper:]' '[:lower:]')
LOG_LEVEL_ENV=$(printf "%s" "${LOG_LEVEL:-info}" | tr '[:upper:]' '[:lower:]')

UVICORN_ARGS="--host ${HOST_ENV} --port ${PORT_ENV} --log-level ${LOG_LEVEL_ENV} --no-access-log"

if [ "$RELOAD_ENV" = "true" ]; then
  UVICORN_ARGS="${UVICORN_ARGS} --reload"
fi

exec python -m uvicorn main:app ${UVICORN_ARGS}
