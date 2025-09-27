import { Outlet } from "react-router-dom";
import AdminSidebar from "./AdminSidebar";
import ScaledContainer from "./ScaledContainer";

export default function AdminLayout() {
  return (
    <ScaledContainer>
      <div className="relative flex flex-col w-full h-full overflow-hidden">
        <div className="flex flex-1 overflow-hidden">
          <AdminSidebar />
          <main className="flex-1 overflow-hidden p-4">
            <Outlet />
          </main>
        </div>
      </div>
    </ScaledContainer>
  );
}
