// src/layout/index.tsx
import { Outlet } from "react-router-dom";
import { Header } from "./header";
import ScaledContainer from "./ScaledContainer";

export default function Layout() {
  return (
    <ScaledContainer>
      <div className="relative flex h-full flex-col w-full overflow-hidden">
        <div>
          <Header />
        </div>
        <main className="flex-1 overflow-hidden">
          <Outlet />
        </main>
      </div>
    </ScaledContainer>
  );
}
