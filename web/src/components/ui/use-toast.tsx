import * as React from "react";
import {
  Toast,
  ToastProvider,
  ToastViewport,
  ToastTitle,
  ToastDescription,
  ToastClose,
} from "./toast";

export type ToastMessage = {
  id?: string;
  title?: string;
  description?: string;
  variant?: "default" | "success" | "error" | "loading";
  duration?: number; // ms; ignored while variant === 'loading'
};

interface ToastContextValue {
  /** Create a toast. Returns the toast id (generated if absent). */
  toast: (msg: ToastMessage) => string;
  /** Update an existing toast by id (e.g. switch from loading to success). */
  updateToast: (id: string, msg: Partial<Omit<ToastMessage, "id">>) => void;
  /** Dismiss a toast early. */
  dismiss: (id: string) => void;
}

const ToastContext = React.createContext<ToastContextValue | null>(null);

export const ToastContextProvider: React.FC<{ children: React.ReactNode }> = ({
  children,
}) => {
  const [toasts, setToasts] = React.useState<ToastMessage[]>([]);
  const timersRef = React.useRef<Record<string, number>>({});

  const remove = React.useCallback((id: string) => {
    setToasts((prev) => prev.filter((t) => t.id !== id));
    if (timersRef.current[id]) {
      window.clearTimeout(timersRef.current[id]);
      delete timersRef.current[id];
    }
  }, []);

  const toast = React.useCallback(
    (message: ToastMessage) => {
      const id = message.id || Math.random().toString(36).slice(2);
      setToasts((prev) => {
        // If id already exists, replace instead of duplicate
        const exists = prev.some((t) => t.id === id);
        if (exists) {
          return prev.map((t) => (t.id === id ? { ...t, ...message, id } : t));
        }
        return [...prev, { ...message, id }];
      });
      // Clear any previous timer for reuse scenarios
      if (timersRef.current[id]) {
        window.clearTimeout(timersRef.current[id]);
        delete timersRef.current[id];
      }
      // "loading" variant stays until updated / dismissed
      if (message.variant !== "loading") {
        const duration = message.duration ?? 3000;
        timersRef.current[id] = window.setTimeout(() => remove(id), duration);
      }
      return id;
    },
    [remove]
  );

  const updateToast = React.useCallback(
    (id: string, patch: Partial<Omit<ToastMessage, "id">>) => {
      setToasts((prev) =>
        prev.map((t) => (t.id === id ? { ...t, ...patch, id } : t))
      );
      // Reset timer logic
      if (timersRef.current[id]) {
        window.clearTimeout(timersRef.current[id]);
        delete timersRef.current[id];
      }
      const variant = patch.variant; // may be undefined
      // Find current to determine effective variant after patch
      const effective = ((): ToastMessage["variant"] => {
        const current = toastsRef.current.find((t) => t.id === id);
        return (variant ||
          current?.variant ||
          "default") as ToastMessage["variant"];
      })();
      if (effective !== "loading") {
        const duration = patch.duration ?? 3000;
        timersRef.current[id] = window.setTimeout(() => remove(id), duration);
      }
    },
    [remove]
  );

  // Keep a ref of latest toasts for updateToast variant resolution
  const toastsRef = React.useRef<ToastMessage[]>([]);
  React.useEffect(() => {
    toastsRef.current = toasts;
  }, [toasts]);

  // Cleanup all timers on unmount
  React.useEffect(() => {
    return () => {
      Object.values(timersRef.current).forEach((t) => window.clearTimeout(t));
      timersRef.current = {};
    };
  }, []);

  return (
    <ToastContext.Provider value={{ toast, updateToast, dismiss: remove }}>
      <ToastProvider>
        {children}
        {toasts.map((t) => {
          return (
            <Toast
              key={t.id}
              variant={t.variant}
              open
              onOpenChange={(o) => !o && t.id && remove(t.id)}
            >
              {t.title && (
                <div className="flex items-center gap-2">
                  {t.variant === "loading" && (
                    <span className="h-3 w-3 animate-spin rounded-full border-2 border-white/40 border-t-white" />
                  )}
                  <ToastTitle>{t.title}</ToastTitle>
                </div>
              )}
              {t.description && (
                <ToastDescription>{t.description}</ToastDescription>
              )}
              <ToastClose />
            </Toast>
          );
        })}
        <ToastViewport />
      </ToastProvider>
    </ToastContext.Provider>
  );
};

export function useToast() {
  const ctx = React.useContext(ToastContext);
  if (!ctx) throw new Error("useToast 必须在 ToastContextProvider 内使用");
  return ctx;
}
