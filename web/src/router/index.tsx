import { createBrowserRouter } from "react-router-dom";

import Layout from "@/layout";
import Home from "@/views/home";
import { lazy, Suspense, useEffect } from "react";
import LoadingSkeleton from "@/components/LoadingSkeleton";
import ErrorBoundary from "@/components/ErrorBoundary";
import { useAuth } from "@/context/AuthContext";
import { Navigate } from "react-router-dom";

const AdminLayout = lazy(() => import("@/layout/AdminLayout"));
const Admin = lazy(() => import("@/views/admin/dashboard"));
const AdminUsers = lazy(() => import("@/views/admin/users"));
const AdminModels = lazy(() => import("@/views/admin/models"));
const Workspace = lazy(() => import("@/views/workspace"));
const Viewer = lazy(() => import("@/views/viewer"));

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

const ProtectedAdmin: React.FC<{ children: React.ReactElement }> = ({
  children,
}) => {
  const { isAuthenticated, openLoginModal, user } = useAuth();

  useEffect(() => {
    if (!isAuthenticated) {
      openLoginModal();
    }
  }, [isAuthenticated]);

  if (!isAuthenticated) return <Navigate to="/home" replace />;
  if (user?.role !== "admin") return <Navigate to="/home" replace />;

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
            <ErrorBoundary>
              <Suspense fallback={<LoadingSkeleton variant="page" />}>
                <Workspace />
              </Suspense>
            </ErrorBoundary>
          </Protected>
        ),
      },
      {
        path: "viewer",
        element: (
          <ErrorBoundary>
            <Suspense fallback={<LoadingSkeleton variant="page" />}>
              <Viewer />
            </Suspense>
          </ErrorBoundary>
        ),
      },
      {
        path: "admin",
        element: (
          <ProtectedAdmin>
            <ErrorBoundary>
              <Suspense fallback={<LoadingSkeleton variant="panel" />}>
                <AdminLayout />
              </Suspense>
            </ErrorBoundary>
          </ProtectedAdmin>
        ),
        children: [
          {
            index: true,
            element: (
              <ErrorBoundary>
                <Suspense fallback={<LoadingSkeleton variant="panel" />}>
                  {" "}
                  <Admin />{" "}
                </Suspense>
              </ErrorBoundary>
            ),
          },
          {
            path: "users",
            element: (
              <ErrorBoundary>
                <Suspense fallback={<LoadingSkeleton variant="panel" />}>
                  {" "}
                  <AdminUsers />{" "}
                </Suspense>
              </ErrorBoundary>
            ),
          },
          {
            path: "models",
            element: (
              <ErrorBoundary>
                <Suspense fallback={<LoadingSkeleton variant="panel" />}>
                  {" "}
                  <AdminModels />{" "}
                </Suspense>
              </ErrorBoundary>
            ),
          },
        ],
      },
    ],
  },
]);

export default router;
