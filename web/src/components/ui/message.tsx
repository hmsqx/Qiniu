import React, {
  createContext,
  useCallback,
  useContext,
  useMemo,
  useState,
} from "react";
import { CheckCircle2, Info, AlertTriangle, XCircle } from "lucide-react";
import { cn } from "@/lib/utils";

type MessageType = "info" | "success" | "warning" | "error";

export type MessageOptions = {
  content: React.ReactNode; // 文案或自定义节点（由调用方传递）
  type?: MessageType;
  duration?: number; // 毫秒，默认 2000；传 0 则不自动关闭
};

type InternalMsg = MessageOptions & { id: number; createdAt: number };

type Ctx = {
  show: (opts: MessageOptions) => { close: () => void };
};

const MessageCtx = createContext<Ctx | null>(null);

export const MessageProvider: React.FC<{ children: React.ReactNode }> = ({
  children,
}) => {
  const [msgs, setMsgs] = useState<InternalMsg[]>([]);

  const show = useCallback((opts: MessageOptions) => {
    const id = Date.now() + Math.random();
    const msg: InternalMsg = {
      id,
      createdAt: Date.now(),
      type: opts.type || "info",
      content: opts.content,
      duration: opts.duration ?? 2000,
    };
    setMsgs((prev) => [...prev, msg]);

    let timer: number | undefined;
    if (msg.duration && msg.duration > 0) {
      timer = window.setTimeout(() => {
        setMsgs((prev) => prev.filter((m) => m.id !== id));
      }, msg.duration);
    }

    return {
      close: () => {
        if (timer) window.clearTimeout(timer);
        setMsgs((prev) => prev.filter((m) => m.id !== id));
      },
    };
  }, []);

  const value = useMemo<Ctx>(() => ({ show }), [show]);

  return (
    <MessageCtx.Provider value={value}>
      {children}
      {/* Portal-like 容器：居中显示 */}
      <div
        className={cn(
          "pointer-events-none fixed inset-0 z-50 flex items-center justify-center",
          "px-4 py-10"
        )}
      >
        <div className="flex flex-col gap-3 w-full max-w-md">
          {msgs.map((m) => (
            <div
              key={m.id}
              className={cn(
                "pointer-events-auto mx-auto w-full rounded-xl border shadow-lg",
                "px-4 py-3 flex items-start gap-3",
                "bg-background/95 backdrop-blur supports-[backdrop-filter]:bg-background/80",
                "border-border",
                m.type === "success" &&
                  "text-emerald-600 dark:text-emerald-400",
                m.type === "info" && "text-sky-600 dark:text-sky-400",
                m.type === "warning" && "text-amber-600 dark:text-amber-400",
                m.type === "error" && "text-red-600 dark:text-red-400"
              )}
            >
              <span className="mt-0.5">
                {m.type === "success" && <CheckCircle2 className="h-5 w-5" />}
                {m.type === "info" && <Info className="h-5 w-5" />}
                {m.type === "warning" && <AlertTriangle className="h-5 w-5" />}
                {m.type === "error" && <XCircle className="h-5 w-5" />}
              </span>
              <div className="text-sm text-foreground">
                {typeof m.content === "string" ? (
                  <span>{m.content}</span>
                ) : (
                  m.content
                )}
              </div>
            </div>
          ))}
        </div>
      </div>
    </MessageCtx.Provider>
  );
};

export function useMessage() {
  const ctx = useContext(MessageCtx);
  if (!ctx) throw new Error("useMessage 必须在 MessageProvider 内使用");
  return ctx;
}
