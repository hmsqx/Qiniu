# 读取环境变量
def load_env(file_path=".env"):
    env_vars = {}
    try:
        with open(file_path) as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                if "=" not in line:
                    continue  # 跳过格式不正确的行
                key, value = line.split("=", 1)
                env_vars[key] = value
    except FileNotFoundError:
        print(f"Warning: Environment file '{file_path}' not found. Please create it with required API keys.")
        # 返回空字典，让调用者处理
        return {}
    except Exception as e:
        print(f"Error reading environment file '{file_path}': {str(e)}")
        return {}
    return env_vars


