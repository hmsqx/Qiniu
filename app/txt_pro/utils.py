# 读取环境变量
def load_env(file_path=".env"):
    env_vars = {}
    with open(file_path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            key, value = line.split("=", 1)
            env_vars[key] = value
    return env_vars


