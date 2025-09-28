#!/bin/sh
set -e

# Seed models from a host/built-in directory into the writable models volume
# Env Vars:
#   MODEL_FS_BASE_DIR   - Destination models dir (defaults to /var/www/models)
#   SEED_MODELS_DIR     - Source seed dir (defaults to /opt/seed-models)
#   ALWAYS_SEED_MODELS  - If set to 1, always attempt to seed on start (default 0)
#   SEED_OVERWRITE      - If set to 1, overwrite existing files when seeding (default 0)

MODEL_DIR=${MODEL_FS_BASE_DIR:-/var/www/models}
SEED_DIR=${SEED_MODELS_DIR:-/opt/seed-models}
ALWAYS_SEED=${ALWAYS_SEED_MODELS:-0}
OVERWRITE=${SEED_OVERWRITE:-0}

mkdir -p "$MODEL_DIR"

seed_if_needed() {
  if [ ! -d "$SEED_DIR" ]; then
    return 0
  fi

  # Check if seed dir has any content
  if [ -z "$(find "$SEED_DIR" -mindepth 1 -print -quit 2>/dev/null)" ]; then
    return 0
  fi

  # Determine if destination is empty
  DEST_EMPTY=0
  if [ -z "$(find "$MODEL_DIR" -mindepth 1 -print -quit 2>/dev/null)" ]; then
    DEST_EMPTY=1
  fi

  if [ "$ALWAYS_SEED" = "1" ] || [ "$DEST_EMPTY" = "1" ]; then
    echo "[entrypoint] Seeding models from $SEED_DIR to $MODEL_DIR (overwrite=$OVERWRITE)" >&2
    if [ "$OVERWRITE" = "1" ]; then
      # Copy and overwrite existing files
      cp -a "$SEED_DIR"/. "$MODEL_DIR"/
    else
      # -n: do not overwrite existing files
      cp -an "$SEED_DIR"/. "$MODEL_DIR"/
    fi
  else
    echo "[entrypoint] Destination $MODEL_DIR not empty; skipping seeding. Set ALWAYS_SEED_MODELS=1 to force." >&2
  fi
}

seed_if_needed

echo "[entrypoint] Starting: $*" >&2
exec "$@"
