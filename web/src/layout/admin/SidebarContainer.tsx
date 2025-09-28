import { cn } from "@/lib/utils";

type Props = {
  variant?: "inline" | "drawer";
  className?: string;
  children?: React.ReactNode;
};

export default function SidebarContainer({
  variant = "inline",
  className,
  children,
}: Props) {
  const containerCls =
    variant === "inline"
      ? "w-56 hidden md:flex flex-col bg-gradient-to-b from-background/70 to-background/40 backdrop-blur-xl border-r border-border/30 supports-[backdrop-filter]:bg-background/55"
      : "w-64 flex flex-col h-full bg-gradient-to-b from-background to-background/95 border-r border-border/30 backdrop-blur-xl";
  return <aside className={cn(containerCls, className)}>{children}</aside>;
}
