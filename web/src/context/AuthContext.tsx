import React, { createContext, useContext, useEffect, useState } from "react";
import { setAuthToken } from "@/utils/request";
import { loginApi, registerApi, meApi, logoutApi } from "@/api/auth";

type User = {
  id: string;
  username: string;
  role?: string | null;
  tokenCount?: number;
  sessionToken?: string | null;
  email?: string | null;
  avatar?: string | null;
};

type AuthContextType = {
  user: User | null;
  isAuthenticated: boolean;
  login: (usernameOrEmail: string, password: string) => Promise<User>;
  register: (
    username: string,
    email: string,
    password: string
  ) => Promise<User>;
  logout: () => Promise<void>;
  openLoginModal: () => void;
  closeLoginModal: () => void;
  loginModalOpen: boolean;
  refreshUser: () => Promise<User | null>;
};

const AuthContext = createContext<AuthContextType | undefined>(undefined);

const AUTH_STORAGE = "gen3d_auth";

export const AuthProvider: React.FC<{ children: React.ReactNode }> = ({
  children,
}) => {
  const [user, setUser] = useState<User | null>(() => {
    try {
      const raw = localStorage.getItem(AUTH_STORAGE);
      if (!raw) return null;
      const parsed = JSON.parse(raw);
      if (parsed && typeof parsed === "object") {
        if (parsed.token_count && !parsed.tokenCount) {
          const num =
            typeof parsed.token_count === "number"
              ? parsed.token_count
              : parseInt(parsed.token_count, 10);
          parsed.tokenCount = isNaN(num) ? undefined : num;
        }
        parsed.id = parsed.id || parsed.userId || parsed.username;
        return parsed as User;
      }
      return null;
    } catch (e) {
      return null;
    }
  });

  const [loginModalOpen, setLoginModalOpen] = useState(false);

  useEffect(() => {
    if (user) {
      try {
        localStorage.setItem(AUTH_STORAGE, JSON.stringify(user));
      } catch (e) {}
      setAuthToken(user.sessionToken || undefined);
    } else {
      localStorage.removeItem(AUTH_STORAGE);
      setAuthToken(undefined);
    }
  }, [user]);

  const login = async (usernameOrEmail: string, password: string) => {
    const body = { usernameOrEmail, password };
    const resp = await loginApi(body).catch((e) => {
      throw new Error(e?.response?.data?.message || e?.message || "登录失败");
    });

    const ok =
      (resp as any)?.status === "success" || (resp as any)?.code === 200;
    const data = (resp as any)?.data || resp;

    if (ok || data?.sessionToken) {
      const baseUser: User = {
        id: data?.userId || usernameOrEmail,
        username: data?.username || usernameOrEmail,
        email: data?.email || null,
        sessionToken: data?.sessionToken || null,
        avatar: data?.avatar || null,
      };

      setAuthToken(baseUser.sessionToken || undefined);
      setUser(baseUser);

      try {
        const meResp = await meApi();
        const me = (meResp as any)?.data || meResp;
        if (me) {
          const enriched: User = {
            id: me.userId || me.id || baseUser.id,
            username: me.username || baseUser.username,
            role: me.role ?? null,
            tokenCount:
              typeof me.token_count === "number"
                ? me.token_count
                : parseInt(me.token_count, 10) || undefined,
            email: me.email ?? baseUser.email ?? null,
            sessionToken: baseUser.sessionToken,
            avatar: baseUser.avatar || me.avatar || null,
          };
          setUser(enriched);
          return enriched;
        }
      } catch (e) {
        console.warn("获取当前用户信息失败 (/auth/me)", e);
      }
      return baseUser;
    }

    throw new Error((resp as any)?.message || "登录失败");
  };

  const register = async (
    username: string,
    email: string,
    password: string
  ) => {
    const body = { username, email, password };
    const resp = await registerApi(body).catch((e) => {
      throw new Error(e?.response?.data?.message || e?.message || "注册失败");
    });

    const ok =
      (resp as any)?.status === "success" || (resp as any)?.code === 200;
    if (ok) {
      const u = await login(username, password);
      return u;
    }

    throw new Error((resp as any)?.message || "注册失败");
  };

  const logout = async () => {
    const sessionToken = user?.sessionToken;
    let logoutError: Error | null = null;

    if (sessionToken) {
      try {
        const resp = await logoutApi(sessionToken);
        const ok =
          (resp as any)?.status === "success" ||
          (resp as any)?.code === 200 ||
          (resp as any)?.code === 0 ||
          typeof (resp as any)?.code === "undefined";

        if (!ok) {
          logoutError = new Error(
            (resp as any)?.message || "退出登录失败，请稍后重试"
          );
        }
      } catch (error: any) {
        console.warn("logout 调用失败", error);
        logoutError = new Error(
          error?.response?.data?.message ||
            error?.message ||
            "退出登录失败，请稍后重试"
        );
      }
    }

    setUser(null);
    setAuthToken(undefined);

    if (logoutError) {
      throw logoutError;
    }
  };

  const openLoginModal = () => setLoginModalOpen(true);
  const closeLoginModal = () => setLoginModalOpen(false);

  const refreshUser = async (): Promise<User | null> => {
    if (!user?.sessionToken) return null;
    try {
      const meResp = await meApi();
      const me = (meResp as any)?.data || meResp;
      if (!me) return user;
      const merged: User = {
        id: me.userId || me.id || user.id,
        username: me.username || user.username,
        role: me.role ?? user.role ?? null,
        tokenCount:
          typeof me.token_count === "number"
            ? me.token_count
            : parseInt(me.token_count, 10) || user.tokenCount,
        email: me.email ?? user.email ?? null,
        sessionToken: user.sessionToken,
        avatar: user.avatar || me.avatar || null,
      };
      setUser(merged);
      return merged;
    } catch (e) {
      console.warn("refreshUser 调用失败", e);
      return user;
    }
  };

  useEffect(() => {
    if (
      user?.sessionToken &&
      (user?.tokenCount === undefined || user?.role === undefined)
    ) {
      refreshUser();
    }
  }, []);

  return (
    <AuthContext.Provider
      value={{
        user,
        isAuthenticated: !!user,
        login,
        register,
        logout,
        openLoginModal,
        closeLoginModal,
        loginModalOpen,
        refreshUser,
      }}
    >
      {children}
    </AuthContext.Provider>
  );
};

export const useAuth = () => {
  const ctx = useContext(AuthContext);
  if (!ctx) throw new Error("useAuth must be used within AuthProvider");
  return ctx;
};

export default AuthContext;
