import React from "react";
import ReactDOM from "react-dom/client";
import { RouterProvider } from "react-router-dom";
import router from "./router/index"; // 导入我们创建的路由实例
import "./index.css"; // 你的全局 CSS
import { AuthProvider } from "@/context/AuthContext";
import LoginModal from "@/components/LoginModal";
import { MessageProvider } from "@/components/ui/message";

ReactDOM.createRoot(document.getElementById("root")!).render(
  <React.StrictMode>
    <AuthProvider>
      <MessageProvider>
        <RouterProvider router={router} />
        <LoginModal />
      </MessageProvider>
    </AuthProvider>
  </React.StrictMode>
);
