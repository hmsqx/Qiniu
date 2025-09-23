import React, { createContext, useContext, useEffect, useState } from "react";
import { setAuthToken } from "@/utils/request";
import { loginApi, registerApi } from "@/api/auth";

type User = {
  id: string;
  username: string;
  avatar?: string | null;
  sessionToken?: string | null;
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
      const u: User = {
        id: data?.userId || usernameOrEmail,
        username: data?.username || usernameOrEmail,
        avatar: `https://i.pravatar.cc/150?u=${encodeURIComponent(
          data?.username || usernameOrEmail
        )}`,
        sessionToken: data?.sessionToken || null,
      };
      setUser(u);
      setAuthToken(u.sessionToken || undefined);
      return u;
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
