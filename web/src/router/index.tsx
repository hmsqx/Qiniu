import { createBrowserRouter } from "react-router-dom";

import Layout from "@/layout";
import Home from "@/views/home";
import Workspace from "@/views/workspace";
import { useEffect } from "react";
import { useAuth } from "@/context/AuthContext";
import { Navigate } from "react-router-dom";
import Viewer from "@/views/viewer";

// small Protected wrapper component used in routes
const Protected: React.FC<{ children: React.ReactElement }> = ({
  children,
}) => {
  const { isAuthenticated, openLoginModal } = useAuth();

  useEffect(() => {
    if (!isAuthenticated) {
      openLoginModal();
    }
  }, [isAuthenticated]);

  if (!isAuthenticated) return <Navigate to="/home" replace />;

  return children;
};

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
        element: (
          <Protected>
            <Workspace />
          </Protected>
        ),
      },
      {
        path: "viewer",
        // 公共预览页，不需要鉴权
        element: <Viewer />,
      },
    ],
  },
]);

export default router;
