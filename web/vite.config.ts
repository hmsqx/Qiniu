import path from "path";
import tailwindcss from "@tailwindcss/vite";
import react from "@vitejs/plugin-react";
import { defineConfig, loadEnv } from "vite";

export default ({ mode }: { mode: string }) => {
  const env = loadEnv(mode, process.cwd());

  const proxyTarget = "http://47.120.8.25:8080";
  const proxyTarget2 = env.VITE_PROXY_TARGET2 || "http://47.120.8.25:8090";
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
        "/model": {
          target: "http://8.155.35.99",
          changeOrigin: true,
          rewrite: (p) => p.replace(/^\/model/, "models"),
        },
        "/llm": {
          target: proxyTarget2,
          changeOrigin: true,
          rewrite: (p) => p.replace(/^\/llm/, ""),
        },
      },
    },
    define: {},
  });
};
