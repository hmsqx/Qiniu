import path from "path";
import { fileURLToPath } from "url";
import tailwindcss from "@tailwindcss/vite";
import react from "@vitejs/plugin-react";
import { defineConfig, loadEnv } from "vite";

const __dirnameESM =
  typeof __dirname === "string"
    ? __dirname
    : path.dirname(fileURLToPath(import.meta.url));

export default ({ mode }: { mode: string }) => {
  const env = loadEnv(mode, process.cwd());

  const proxyTarget = env.VITE_API_UPSTREAM || "http://localhost:8080";
  const proxyTarget2 = env.VITE_LLM_UPSTREAM || "http://localhost:8090";

  const isProd = mode === "production";

  return defineConfig({
    plugins: [react(), tailwindcss()],
    resolve: {
      alias: {
        "@": path.resolve(__dirnameESM, "./src"),
      },
      dedupe: ["react", "react-dom"],
    },
    server: {
      host: true,
      proxy: {
        "/api": {
          target: proxyTarget,
          changeOrigin: true,
        },
        "/models": {
          target: proxyTarget,
          changeOrigin: true,
        },
        "/llm": {
          target: proxyTarget2,
          changeOrigin: true,
          rewrite: (p) => p.replace(/^\/llm/, ""),
        },
      },
    },
    optimizeDeps: {
      include: [
        "react",
        "react-dom",
        "react-router-dom",
        "framer-motion",
        "motion",
        "three",
        "three-stdlib",
        "recharts",
        "zod",
        "react-hook-form",
      ],
    },
    build: {
      target: "es2020",
      cssCodeSplit: true,
      sourcemap: isProd ? false : true,
      reportCompressedSize: true,
      chunkSizeWarningLimit: 1200,
      rollupOptions: {
        output: {
          manualChunks(id) {
            if (!id.includes("node_modules")) return undefined;
            if (/react|react-dom|react-router/.test(id)) return "vendor-react";
            if (/radix-ui|lucide-react/.test(id)) return "vendor-ui";
            if (/three(?!-stdlib)|three-stdlib/.test(id)) return "vendor-three";
            if (/recharts/.test(id)) return "vendor-charts";
            if (/zod|react-hook-form/.test(id)) return "vendor-forms";
            return "vendor";
          },
        },
      },
      minify: "esbuild",
    },
    esbuild: isProd
      ? {
          drop: ["console", "debugger"],
        }
      : undefined,
    define: {
      "process.env.NODE_ENV": JSON.stringify(mode),
      __APP_VERSION__: JSON.stringify(
        process.env.npm_package_version ?? "0.0.0"
      ),
    },
  });
};
