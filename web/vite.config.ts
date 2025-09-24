import path from "path";
import tailwindcss from "@tailwindcss/vite";
import react from "@vitejs/plugin-react";
import { defineConfig, loadEnv } from "vite";

export default ({ mode }: { mode: string }) => {
  const env = loadEnv(mode, process.cwd());

  const proxyTarget = env.VITE_PROXY_TARGET || "http://localhost:3000";
  const cosTarget =
    env.VITE_COS_TARGET ||
    "https://hunyuan-prod-1258344699.cos.ap-guangzhou.tencentcos.cn";

  return defineConfig({
    plugins: [react(), tailwindcss()],
    resolve: {
      alias: {
        "@": path.resolve(__dirname, "./src"),
      },
    },
    server: {
      proxy: {
        "/api": {
          target: proxyTarget,
          changeOrigin: true,
          rewrite: (p) => p.replace(/^\/api/, ""),
        },
        "/cos": {
          target: cosTarget,
          changeOrigin: true,
          secure: true,
          // Map /cos/<path> -> cosTarget/<path>
          rewrite: (p) => p.replace(/^\/cos/, ""),
        },
      },
    },

    define: {},
  });
};
