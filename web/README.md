## 本地 Mock 说明

已添加 `src/api/users.ts` 的本地模拟数据：

特性：

- 默认启用（开发更便捷），通过环境变量 `VITE_USE_MOCK=false` 关闭。
- 稳定伪随机生成 123 条用户数据（id 1~123），支持分页与模糊搜索（username / email / role）。
- 支持参数：`page`、`pageSize`、`keyword`。
- 模拟 300ms 网络延迟。

使用方式：

1. 直接在页面中使用 `useUsers()`（已接入）。
2. 若需要真实后端接口，在 `.env.local` 或 `.env.production` 中加入：
   ```bash
   VITE_USE_MOCK=false
   ```
3. 然后确保后端提供 GET `/api/admin/users`，返回格式：
   ```json
   {
     "list": [
       {
         "id": "1",
         "username": "user_001",
         "email": "user_001@example.com",
         "role": "member",
         "createdAt": "2025-01-01T00:00:00.000Z"
       }
     ],
     "total": 123
   }
   ```

扩展：

- 如需更复杂的交互（新增、编辑、删除），可在 mock 分支中维护本地数组并导出对应函数。
- 如果后续需要更贴近真实网络层的拦截，可引入 [MSW](https://mswjs.io/) 在 `src/mocks/browser.ts` 中注册，再在 `main.tsx` 启动时按条件启动 worker。

环境变量建议：

```bash
# .env.development （默认开启 mock）
VITE_USE_MOCK=true

# .env.production （关闭）
VITE_USE_MOCK=false
```

如需扩展其它实体的 mock，可复制 `users.ts` 的结构。
