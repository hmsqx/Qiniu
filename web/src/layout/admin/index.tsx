import { useState } from "react";
import { Outlet } from "react-router-dom";
import AdminSidebar from "./AdminSidebar";
import AdminTopBar from "./AdminTopBar";
import AdminSidebarDrawer from "./AdminSidebarDrawer";

export default function AdminLayout() {
  const [open, setOpen] = useState(false);
  return (
    <div className="relative flex flex-col w-full h-full overflow-hidden">
      <AdminTopBar onOpenSidebar={() => setOpen(true)} />
      <div className="flex flex-1 overflow-hidden">
        {/* 左侧内联侧栏（>=md 显示） */}
        <AdminSidebar variant="inline" />
        {/* 主内容区域可滚动 */}
        <main className="flex-1 overflow-auto p-4">
          <Outlet />
        </main>
      </div>
      <AdminSidebarDrawer open={open} onOpenChange={setOpen} />
    </div>
  );
}
