import React, {
  createContext,
  useContext,
  useEffect,
  useRef,
  useState,
} from "react";
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
      if (!parsed || typeof parsed !== "object") return null;

      parsed.id = parsed.id || parsed.userId || parsed.username;
      return parsed as User;
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

    const { data, status, code, message } = resp as any;
    const ok = status === "success" || code === 200;
    if (!ok || !data?.sessionToken) {
      throw new Error(message || "登录失败");
    }

    const baseUser: User = {
      id: data.userId || usernameOrEmail,
      username: data.username || usernameOrEmail,
      email: data.email ?? null,
      sessionToken: data.sessionToken ?? null,
      avatar: data.avatar ?? null,
    };

    setAuthToken(baseUser.sessionToken || undefined);
    setUser(baseUser);

    try {
      const meResp = await meApi();
      const me = (meResp as any)?.data;
      if (me) {
        const enriched: User = {
          id: me.userId || baseUser.id,
          username: me.username || baseUser.username,
          role: me.role ?? null,
          tokenCount: Number(me.token_count),
          email: me.email ?? baseUser.email ?? null,
          sessionToken: baseUser.sessionToken,
          avatar: me.avatar ?? baseUser.avatar ?? null,
        };
        setUser(enriched);
        return enriched;
      }
    } catch (e) {
      console.warn("获取当前用户信息失败 (/auth/me)", e);
    }
    return baseUser;
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
    const { status, code, message } = resp as any;
    const ok = status === "success" || code === 200;
    if (!ok) throw new Error(message || "注册失败");
    return await login(username, password);
  };

  const logout = async () => {
    const sessionToken = user?.sessionToken;
    let logoutError: Error | null = null;

    if (sessionToken) {
      try {
        await logoutApi(sessionToken);
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
      const me = (meResp as any)?.data;
      if (!me) return user;
      const merged: User = {
        id: me.userId || user.id,
        username: me.username || user.username,
        role: me.role ?? user.role ?? null,
        tokenCount: Number(me.token_count ?? user.tokenCount),
        email: me.email ?? user.email ?? null,
        sessionToken: user.sessionToken,
        avatar: me.avatar ?? user.avatar ?? null,
      };
      setUser(merged);
      return merged;
    } catch (e) {
      // 如果 session 已无效，清理本地登录状态
      const status = (e as any)?.response?.status;
      if (status === 401 || status === 403) {
        setUser(null);
        setAuthToken(undefined);
        try {
          localStorage.removeItem(AUTH_STORAGE);
        } catch {}
        return null;
      }
      console.warn("refreshUser 调用失败", e);
      return user;
    }
  };

  const didInitRef = useRef(false);
  useEffect(() => {
    // 页面首次打开时，如果存在会话，则强制拉取一次最新用户信息
    // 防止 React 18 严格模式下开发环境触发两次
    if (didInitRef.current) return;
    didInitRef.current = true;
    if (user?.sessionToken) {
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
