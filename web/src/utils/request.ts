import axios from "axios";
import type { AxiosInstance, AxiosRequestConfig, AxiosResponse } from "axios";

const VITE_API_BASE =
  (import.meta.env as Record<string, any>).VITE_API_BASE || "/";

const instance: AxiosInstance = axios.create({
  baseURL: VITE_API_BASE,
  timeout: 30000,
  headers: {
    "Content-Type": "application/json",
  },
});

instance.interceptors.request.use(
  (config: AxiosRequestConfig | any) => {
    try {
      const raw = localStorage.getItem("gen3d_auth");
      if (raw) {
        const parsed = JSON.parse(raw) as { sessionToken?: string };
        const token = parsed?.sessionToken;
        if (token) {
          config.headers = config.headers || {};
          (config.headers as Record<string, any>)["Session-Token"] = token;

          if ((config.headers as Record<string, any>)["Authorization"]) {
            delete (config.headers as Record<string, any>)["Authorization"];
          }
        }
      }
    } catch (_) {
      // 忽略存储解析错误
    }
    return config;
  },
  (error: any) => Promise.reject(error)
);

export function setAuthToken(token?: string | null) {
  if (token) {
    instance.defaults.headers.common["Session-Token"] = token;
    // 清理旧的 Authorization
    delete instance.defaults.headers.common["Authorization"];
  } else {
    delete instance.defaults.headers.common["Session-Token"];
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

export const callApi = <T = any>(path = "config") => {
  const url = path.startsWith("/") ? path : `/${path}`;
  return http.get<T>(url);
};
