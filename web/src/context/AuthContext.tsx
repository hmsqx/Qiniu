import React, { createContext, useContext, useEffect, useState } from "react";
import { setAuthToken } from "@/utils/request";
import { loginApi, registerApi, meApi } from "@/api/auth";

type User = {
  id: string;
  username: string;
  email?: string | null;
  avatar?: string | null;
  sessionToken?: string | null;
  role?: string | null;
  tokenCount?: number; // camelCase 映射后端 token_count
  // 透传后端返回的其他字段
  [key: string]: any;
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
  logout: () => void;
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
      return raw ? (JSON.parse(raw) as User) : null;
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
      // 基础用户对象（若后续 /api/auth/me 失败可以回退）
      let baseUser: User = {
        id: data?.userId || usernameOrEmail,
        username: data?.username || usernameOrEmail,
        email: data?.email || null,
        avatar: `https://i.pravatar.cc/150?u=${encodeURIComponent(
          data?.username || usernameOrEmail
        )}`,
        sessionToken: data?.sessionToken || null,
      };

      setAuthToken(baseUser.sessionToken || undefined);

      // 立即设定最小用户，保证 UI 及时更新
      setUser(baseUser);

      // 尝试获取完整资料
      try {
        const meResp = await meApi();
        const meData = (meResp as any)?.data || meResp;
        if (meData) {
          const tokenCount =
            meData.tokenCount ?? meData.token_count ?? meData.tokenCounts; // 兼容多种命名
          baseUser = {
            ...baseUser,
            ...meData,
            id: meData.id || meData.userId || baseUser.id,
            username: meData.username || baseUser.username,
            email: meData.email || baseUser.email,
            role: meData.role ?? meData.userRole ?? baseUser.role ?? null,
            tokenCount:
              typeof tokenCount === "number"
                ? tokenCount
                : parseInt(tokenCount, 10) || baseUser.tokenCount,
            sessionToken: baseUser.sessionToken, // 不覆盖 token
          };
          if (!baseUser.avatar) {
            baseUser.avatar =
              meData.avatar ||
              meData.avatarUrl ||
              meData.avatarPath ||
              baseUser.avatar;
          }
          setUser(baseUser);
        }
      } catch (e) {
        // 静默失败，保留基础用户
        console.warn("获取当前用户信息失败 (/api/auth/me)", e);
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
      // Auto-login after successful registration
      const u = await login(username, password);
      return u;
    }

    throw new Error((resp as any)?.message || "注册失败");
  };

  const logout = () => {
    setUser(null);
    setAuthToken(undefined);
  };

  const openLoginModal = () => setLoginModalOpen(true);
  const closeLoginModal = () => setLoginModalOpen(false);

  // 独立刷新用户信息（可在页面其它地方调用）
  const refreshUser = async (): Promise<User | null> => {
    if (!user?.sessionToken) return null;
    try {
      const meResp = await meApi();
      const meData = (meResp as any)?.data || meResp;
      if (!meData) return user;
      const tokenCount =
        meData.tokenCount ?? meData.token_count ?? meData.tokenCounts;
      const merged: User = {
        ...user,
        ...meData,
        id: meData.id || meData.userId || user.id,
        username: meData.username || user.username,
        email: meData.email || user.email,
        role: meData.role ?? meData.userRole ?? user.role ?? null,
        tokenCount:
          typeof tokenCount === "number"
            ? tokenCount
            : parseInt(tokenCount, 10) || user.tokenCount,
        sessionToken: user.sessionToken,
      };
      if (!merged.avatar) {
        merged.avatar =
          meData.avatar || meData.avatarUrl || meData.avatarPath || user.avatar;
      }
      setUser(merged);
      return merged;
    } catch (e) {
      console.warn("refreshUser 调用失败", e);
      return user;
    }
  };

  // 初始挂载时如果有 token 但缺少扩展字段，尝试刷新
  useEffect(() => {
    if (
      user?.sessionToken &&
      (user.tokenCount === undefined || user.role === undefined)
    ) {
      refreshUser();
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
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
