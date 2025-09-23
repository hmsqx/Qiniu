// src/layout/index.tsx
import { Outlet } from "react-router-dom";
import { Header } from "./Header";

export default function Layout() {
  return (
    <div className="relative flex min-h-screen flex-col w-screen h-screen overflow-hidden">
      <Header />
      <main className="flex-1 mt-4 px-4 overflow-auto">
        <Outlet />
      </main>
    </div>
  );
}
