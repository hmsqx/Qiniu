import path from "path";
import tailwindcss from "@tailwindcss/vite";
import react from "@vitejs/plugin-react";
import { defineConfig } from "vite";

export default defineConfig({
  plugins: [react(), tailwindcss()],
  resolve: {
    alias: {
      "@": path.resolve(__dirname, "./src"),
    },
  },
  server: {
    proxy: {
      "/proxy": {
        // 目标服务器地址
        target:
          "https://hunyuan-prod-1258344699.cos.ap-guangzhou.tencentcos.cn",

        changeOrigin: true,
        // 重写请求路径，去掉我们自定义的代理前缀 '/proxy'
        rewrite: (path) => path.replace(/^\/proxy/, ""),
      },
    },
  },
});
