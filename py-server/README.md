## Quickstart（仅包模式）

1. 安装依赖（使用 uv）

```powershell
uv sync
```

2. 配置环境变量

- 复制 `app/.env.example` 为 `app/api.env`，并填写 `DASHSCOPE_API_KEY`。
- 可选：通过环境变量 `APP_ENV_FILE` 指向自定义 env 文件路径。

3. 启动（包模式）

- 从仓库根目录（`D:\qiniu\Qiniu`）运行以下任一命令：

```powershell
# 开发模式（推荐）
uv run uvicorn app.main:app --host 0.0.0.0 --port 8090 --reload

# 或使用 Python 模块方式
python -m app.main
```

4. 健康检查

```powershell
curl http://127.0.0.1:8090/
curl http://127.0.0.1:8090/pro_txt/model-info
```

备注

- 本项目的配置由 `app/core/settings.py` 统一加载，默认读取 `app/api.env`。
- `settings` 不依赖当前工作目录，始终以包路径解析 env 文件。
- 如需在 Linux 上运行 OpenCV，可能需要安装系统库：`libgl1`、`libglib2.0-0`。
