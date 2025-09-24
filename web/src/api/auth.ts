import { post, get } from "@/utils/request";

export type LoginRequest = {
  usernameOrEmail: string;
  password: string;
};

export type RegisterRequest = {
  username: string;
  email: string;
  password: string;
};

export type AuthSuccess = {
  userId?: string;
  username?: string;
  email?: string;
  sessionToken?: string;
};

export type ApiEnvelope<T = any> = {
  code?: number;
  status?: string;
  message?: string;
  data?: T;
};

export async function loginApi(
  body: LoginRequest
): Promise<ApiEnvelope<AuthSuccess> | AuthSuccess> {
  return await post<ApiEnvelope<AuthSuccess> | AuthSuccess>("/api/login", body);
}

export async function registerApi(
  body: RegisterRequest
): Promise<ApiEnvelope<AuthSuccess> | AuthSuccess> {
  return await post<ApiEnvelope<AuthSuccess> | AuthSuccess>(
    "/api/register",
    body
  );
}

// 获取当前登录用户信息
export async function meApi(): Promise<ApiEnvelope<AuthSuccess> | AuthSuccess> {
  return await get<ApiEnvelope<AuthSuccess> | AuthSuccess>("/api/auth/me");
}
