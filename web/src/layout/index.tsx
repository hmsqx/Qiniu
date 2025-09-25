// src/layout/index.tsx
import { Outlet } from "react-router-dom";
import { Header } from "./Header";

export default function Layout() {
  return (
    <div className="relative flex h-screen flex-col w-screen overflow-hidden">
      <div>
        <Header />
      </div>
      <main className="flex-1 overflow-hidden">
        <Outlet />
      </main>
    </div>
  );
}
