import path from "path";
import tailwindcss from "@tailwindcss/vite";
import react from "@vitejs/plugin-react";
import { defineConfig, loadEnv } from "vite";

export default ({ mode }: { mode: string }) => {
  const env = loadEnv(mode, process.cwd());

  const proxyTarget = env.VITE_PROXY_TARGET || "http://8.155.35.99:8080";

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
      },
    },

    define: {},
  });
};
