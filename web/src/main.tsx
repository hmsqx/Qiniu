import React from "react";
import ReactDOM from "react-dom/client";
import { RouterProvider } from "react-router-dom";
import router from "./router/index";
import "./index.css";
import { AuthProvider } from "@/context/AuthContext";
import LoginModal from "@/components/LoginModal";
import { MessageProvider } from "@/components/ui/message";
import { ToastContextProvider } from "@/components/ui/use-toast";

ReactDOM.createRoot(document.getElementById("root")!).render(
  <React.StrictMode>
    <AuthProvider>
      <ToastContextProvider>
        <MessageProvider>
          <RouterProvider router={router} />
          <LoginModal />
        </MessageProvider>
      </ToastContextProvider>
    </AuthProvider>
  </React.StrictMode>
);
