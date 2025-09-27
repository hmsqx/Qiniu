import { NavLink } from "react-router-dom";
import { cn } from "@/lib/utils";
import { adminSideMenus } from "@/constants/adminNav";

export default function AdminSidebar() {
  return (
    <aside className="w-56 hidden md:flex flex-col bg-gradient-to-b from-background/70 to-background/40 backdrop-blur-xl border-r border-border/30 supports-[backdrop-filter]:bg-background/55">
      <nav className="flex-1 overflow-auto py-2 text-sm">
        <ul className="space-y-1.5 px-2">
          {adminSideMenus.map((m) => {
            const Icon = m.icon;
            return (
              <li key={m.to}>
                <NavLink
                  to={m.to}
                  end
                  className={({ isActive }) =>
                    cn(
                      "group flex items-center gap-2 rounded-lg px-3 py-2 text-sm font-medium relative overflow-hidden",
                      "transition-colors",
                      isActive
                        ? "text-foreground bg-white/4 dark:bg-white/5 shadow-[0_0_0_1px_hsl(var(--border)/0.25),0_4px_8px_-2px_rgba(0,0,0,0.35)]"
                        : "text-muted-foreground hover:text-foreground hover:bg-white/3 dark:hover:bg-white/5"
                    )
                  }
                >
                  {({ isActive }) => (
                    <>
                      <span
                        className={cn(
                          "absolute left-0 top-1/2 -translate-y-1/2 w-0.5 h-6 rounded-full bg-primary/80 transition-opacity",
                          isActive
                            ? "opacity-100"
                            : "opacity-0 group-hover:opacity-60"
                        )}
                      />
                      <Icon className="h-4 w-4" />
                      <span className="truncate tracking-tight">{m.label}</span>
                    </>
                  )}
                </NavLink>
              </li>
            );
          })}
        </ul>
      </nav>
    </aside>
  );
}
