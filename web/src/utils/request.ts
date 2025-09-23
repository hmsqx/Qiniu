// 轻量级的类型化 axios 封装
// 用法：
// import http from '@/utils/request'
// const { data } = await http.get<MyType>('/api/foo', { params: { q: 'bar' } })

import axios from "axios";
import type { AxiosInstance, AxiosRequestConfig, AxiosResponse } from "axios";

// 根据 Vite 环境变量确定 baseURL。在开发环境下 `.env` 设置 VITE_API_BASE=/api
// 会命中开发代理。在生产环境下 `.env.production` 可以设置为绝对 URL。
const VITE_API_BASE =
  (import.meta.env as Record<string, any>).VITE_API_BASE || "/";

console.log("使用的 API 基础地址:", VITE_API_BASE);
// 创建基础 axios 实例 — 使用 VITE_API_BASE
const instance: AxiosInstance = axios.create({
  baseURL: VITE_API_BASE,
  // 客户端超时时间设置为 30 秒 — 某些后端操作在开发或高负载下可能需要更长时间
  timeout: 30000,
  headers: {
    "Content-Type": "application/json",
  },
});

// 请求/响应拦截器，可以添加日志或全局错误处理。Token 逻辑可在请求拦截器中后续添加。
instance.interceptors.request.use(
  (config: AxiosRequestConfig | any) => {
    // 从本地存储获取 Authorization 头部（如果有）
    try {
      const raw = localStorage.getItem("gen3d_auth");
      if (raw) {
        const parsed = JSON.parse(raw) as { sessionToken?: string };
        const token = parsed?.sessionToken;
        if (token) {
          config.headers = config.headers || {};
          // 使用标准 'Authorization' 保持头部大小写一致
          (config.headers as Record<string, any>)[
            "Authorization"
          ] = `Bearer ${token}`;
        }
      }
    } catch (_) {
      // 忽略存储解析错误
    }
    return config;
  },
  (error: any) => Promise.reject(error)
);

// 可选：编程式设置 token，如果不希望每次都从存储读取
export function setAuthToken(token?: string | null) {
  if (token) {
    instance.defaults.headers.common["Authorization"] = `Bearer ${token}`;
  } else {
    delete instance.defaults.headers.common["Authorization"];
  }
}

instance.interceptors.response.use(
  (response: AxiosResponse) => response,
  (error: any) => {
    if (error.code === "ECONNABORTED") {
      // timeout
      error.message = "服务器响应时间过长。请检查网络，或稍后重试。";
    } else if (!error.response) {
      error.message =
        error.message ||
        "服务器出错，请稍后重试或联系我们，我们会在第一时间处理。";
    } else if (error.response.status >= 500) {
      error.message = `服务器出错，请稍后重试或联系我们，我们会在第一时间处理。`;
    }

    console.error("HTTP error:", {
      url: error.config?.url,
      method: error.config?.method,
      status: error.response?.status,
      message: error.message,
    });

    return Promise.reject(error);
  }
);

// Generic response type helper: resolves to the `data` property of AxiosResponse
class Http {
  private client: AxiosInstance;

  constructor(client: AxiosInstance) {
    this.client = client;
  }

  async request<T = any>(config: AxiosRequestConfig): Promise<T> {
    const resp = await this.client.request<T>(config);
    return resp.data as T;
  }

  get<T = any>(url: string, config?: AxiosRequestConfig) {
    return this.request<T>({ ...(config || {}), url, method: "GET" });
  }

  post<T = any>(url: string, data?: any, config?: AxiosRequestConfig) {
    return this.request<T>({ ...(config || {}), url, method: "POST", data });
  }

  put<T = any>(url: string, data?: any, config?: AxiosRequestConfig) {
    return this.request<T>({ ...(config || {}), url, method: "PUT", data });
  }

  delete<T = any>(url: string, config?: AxiosRequestConfig) {
    return this.request<T>({ ...(config || {}), url, method: "DELETE" });
  }
}

const http = new Http(instance);

export default http;

// Named exports for convenience
export const get = <T = any>(url: string, config?: AxiosRequestConfig) =>
  http.get<T>(url, config);
export const post = <T = any>(
  url: string,
  data?: any,
  config?: AxiosRequestConfig
) => http.post<T>(url, data, config);
export const put = <T = any>(
  url: string,
  data?: any,
  config?: AxiosRequestConfig
) => http.put<T>(url, data, config);
export const del = <T = any>(url: string, config?: AxiosRequestConfig) =>
  http.delete<T>(url, config);

// Example helper: call a server-provided configuration endpoint
// Usage: const cfg = await callApi<ConfigType>('config')
export const callApi = <T = any>(path = "config") => {
  // ensures leading slash is handled by axios baseURL
  const url = path.startsWith("/") ? path : `/${path}`;
  return http.get<T>(url);
};
