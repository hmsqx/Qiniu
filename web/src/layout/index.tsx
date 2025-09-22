// src/layout/index.tsx
import { Outlet } from "react-router-dom";
import { Header } from "./Header";

export default function Layout() {
  return (
    <div className="relative flex min-h-screen flex-col">
      <Header />
      <main className="flex-1 pt-24 px-4">
        <Outlet />
      </main>
    </div>
  );
}
