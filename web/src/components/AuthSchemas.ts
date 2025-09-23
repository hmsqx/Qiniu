import { z } from "zod";

// 登录 Schema
export const loginSchema = z.object({
  identifier: z.string().min(1, { message: "请输入用户名或邮箱" }),
  password: z.string().min(1, { message: "请输入密码" }),
});

// 注册 Schema
export const registerSchema = z.object({
  username: z.string().min(2, { message: "用户名至少需要2位" }),
  email: z.string().email({ message: "请输入有效的邮箱格式" }),
  password: z.string().min(6, { message: "密码至少需要6位" }),
  confirmPassword: z.string().min(6, { message: "请确认密码" }),
});

// 确认密码必须与密码相同
export const registerSchemaWithConfirm = registerSchema.refine(
  (data) => data.password === data.confirmPassword,
  {
    message: "两次输入的密码不一致",
    path: ["confirmPassword"],
  }
);

export type LoginFormValues = z.infer<typeof loginSchema>;
export type RegisterFormValues = z.infer<typeof registerSchema> & {
  confirmPassword?: string;
};

export default {
  loginSchema,
  registerSchema,
};
