import { createBrowserRouter, Navigate } from "react-router-dom";

import Layout from "@/layout";
import Home from "@/views/home";
import Workspace from "@/views/workspace";

const router = createBrowserRouter([
  {
    path: "/",
    element: <Layout></Layout>,
    children: [
      {
        index: true,
        // 直接提供 JSX 元素
        element: <Navigate to="/home" replace />,
      },
      {
        path: "home",
        // 直接提供 JSX 元素
        element: <Home />,
      },
      {
        path: "workspace",
        // 直接提供 JSX 元素
        element: <Workspace />,
      },
    ],
  },
]);

export default router;
