import React from "react";
import { useForm } from "react-hook-form";
import { zodResolver } from "@hookform/resolvers/zod";
import { Loader2, CircleUserRound } from "lucide-react";

import { Button } from "@/components/ui/button";
import { Label } from "@/components/ui/label";
import { Checkbox } from "@/components/ui/checkbox";
import {
  Dialog,
  DialogContent,
  DialogHeader,
  DialogTitle,
} from "@/components/ui/dialog";
import { Form } from "@/components/ui/form";
import { useAuth } from "@/context/AuthContext";
import AuthInputField from "@/components/AuthInputField";
import {
  loginSchema,
  registerSchemaWithConfirm,
} from "@/components/AuthSchemas";
import type {
  LoginFormValues,
  RegisterFormValues,
} from "@/components/AuthSchemas";

export const LoginModal: React.FC = () => {
  const {
    loginModalOpen,
    closeLoginModal,
    login: authLogin,
    register: authRegister,
  } = useAuth();
  const [mode, setMode] = React.useState<"login" | "register">("login");
  const [serverError, setServerError] = React.useState<string | null>(null);
  const loginForm = useForm<LoginFormValues>({
    resolver: zodResolver(loginSchema),
    defaultValues: { identifier: "", password: "" },
  });

  const registerForm = useForm<RegisterFormValues>({
    resolver: zodResolver(registerSchemaWithConfirm),
    defaultValues: {
      username: "",
      email: "",
      password: "",
      confirmPassword: "",
    },
  });

  const { isSubmitting: loginSubmitting } = loginForm.formState;
  const { isSubmitting: registerSubmitting } = registerForm.formState;

  const toggleMode = () => {
    setMode((prevMode) => (prevMode === "login" ? "register" : "login"));
    loginForm.reset();
    registerForm.reset();
    setServerError(null);
  };

  const handleLogin = async (values: LoginFormValues) => {
    setServerError(null);
    try {
      await authLogin(values.identifier, values.password);
      closeLoginModal();
    } catch (e: any) {
      setServerError(e?.message || "登录失败，请检查您的凭证");
    }
  };

  const handleRegister = async (values: RegisterFormValues) => {
    setServerError(null);
    try {
      await authRegister(values.username, values.email, values.password);
      closeLoginModal();
    } catch (e: any) {
      setServerError(e?.message || "注册失败，请稍后再试");
    }
  };

  return (
    <Dialog
      open={loginModalOpen}
      onOpenChange={(open) => !open && closeLoginModal()}
    >
      <DialogContent className="sm:max-w-md p-6">
        <div className="flex flex-col items-center gap-3">
          <div
            className="flex size-11 shrink-0 items-center justify-center rounded-full border border-border"
            aria-hidden="true"
          >
            <CircleUserRound className="h-5 w-5" />
          </div>
          <DialogHeader className="sm:text-center">
            <DialogTitle>
              {mode === "login" ? "欢迎回来" : "注册新账号"}
            </DialogTitle>
          </DialogHeader>
        </div>

        <div>
          <div className="mt-2">
            {mode === "login" ? (
              <Form {...loginForm}>
                <form
                  onSubmit={loginForm.handleSubmit(handleLogin)}
                  className="space-y-4"
                >
                  {serverError && (
                    <div className="text-sm text-destructive bg-destructive/10 p-3 rounded-md text-center">
                      {serverError}
                    </div>
                  )}

                  <AuthInputField
                    control={loginForm.control}
                    name="identifier"
                    label="邮箱或用户名"
                    placeholder="hi@yourcompany.com"
                  />

                  <AuthInputField
                    control={loginForm.control}
                    name="password"
                    label="密码"
                    placeholder="请输入密码"
                    type="password"
                  />

                  <div className="flex items-center">
                    <Checkbox id="remember-me" />
                    <Label
                      htmlFor="remember-me"
                      className="ml-2 font-normal text-sm text-muted-foreground"
                    >
                      记住我
                    </Label>
                  </div>

                  <Button
                    type="submit"
                    className="w-full"
                    disabled={loginSubmitting}
                  >
                    {loginSubmitting && (
                      <Loader2 className="mr-2 h-4 w-4 animate-spin" />
                    )}
                    登录
                  </Button>
                </form>
              </Form>
            ) : (
              <Form {...registerForm}>
                <form
                  onSubmit={registerForm.handleSubmit(handleRegister)}
                  className="space-y-4"
                >
                  {serverError && (
                    <div className="text-sm text-destructive bg-destructive/10 p-3 rounded-md text-center">
                      {serverError}
                    </div>
                  )}

                  <AuthInputField
                    control={registerForm.control}
                    name="username"
                    label="用户名"
                    placeholder="请输入用户名"
                  />
                  <AuthInputField
                    control={registerForm.control}
                    name="email"
                    label="邮箱"
                    placeholder="请输入邮箱"
                  />
                  <AuthInputField
                    control={registerForm.control}
                    name="password"
                    label="密码"
                    placeholder="请输入密码"
                    type="password"
                    showToggle
                  />
                  <AuthInputField
                    control={registerForm.control}
                    name="confirmPassword"
                    label="确认密码"
                    placeholder="请再次输入密码"
                    type="password"
                    showToggle
                  />

                  <Button
                    type="submit"
                    className="w-full"
                    disabled={registerSubmitting}
                  >
                    {registerSubmitting && (
                      <Loader2 className="mr-2 h-4 w-4 animate-spin" />
                    )}
                    注册并登录
                  </Button>
                </form>
              </Form>
            )}
          </div>

          <div className="relative my-4">
            <div className="absolute inset-0 flex items-center">
              <span className="w-full border-t" />
            </div>
            <div className="relative flex justify-center text-xs uppercase">
              <span className="bg-background px-2 text-muted-foreground">
                或
              </span>
            </div>
          </div>

          <Button
            type="button"
            variant="outline"
            className="w-full"
            onClick={toggleMode}
          >
            {mode === "login" ? "注册新账号" : "已有账号？去登录"}
          </Button>
        </div>
      </DialogContent>
    </Dialog>
  );
};

export default LoginModal;
