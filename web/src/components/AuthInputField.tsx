import React from "react";
import type { Control } from "react-hook-form";
import {
  FormField,
  FormItem,
  FormLabel,
  FormControl,
  FormMessage,
} from "@/components/ui/form";
import { Input } from "@/components/ui/input";
import { Button } from "@/components/ui/button";
import { Eye, EyeOff } from "lucide-react";
import { cn } from "@/lib/utils";

type Props = {
  control: Control<any>;
  name: string;
  label: string;
  placeholder?: string;
  type?: string;
  // if true, show a password visibility toggle
  showToggle?: boolean;
};

export const AuthInputField: React.FC<Props> = ({
  control,
  name,
  label,
  placeholder,
  type = "text",
  showToggle = false,
}) => {
  const [visible, setVisible] = React.useState(false);
  const isPassword = type === "password";

  const inputType =
    isPassword && showToggle ? (visible ? "text" : "password") : type;

  return (
    <FormField
      control={control}
      name={name}
      render={({ field, fieldState }) => (
        <FormItem>
          <FormLabel>{label}</FormLabel>
          <FormControl>
            <div className="relative">
              <Input
                type={inputType}
                placeholder={placeholder}
                {...field}
                className={cn(
                  fieldState.error &&
                    "border-destructive focus-visible:ring-destructive",
                  // padding for toggle button
                  isPassword && showToggle && "pr-10"
                )}
              />
              {isPassword && showToggle && (
                <Button
                  type="button"
                  variant="ghost"
                  size="sm"
                  className="absolute right-1 top-1/2 -translate-y-1/2 h-8 w-8 p-0"
                  onClick={() => setVisible((v) => !v)}
                  aria-label={visible ? "隐藏密码" : "显示密码"}
                >
                  {visible ? (
                    <EyeOff className="h-4 w-4" />
                  ) : (
                    <Eye className="h-4 w-4" />
                  )}
                </Button>
              )}
            </div>
          </FormControl>
          <FormMessage className="h-4" />
        </FormItem>
      )}
    />
  );
};

export default AuthInputField;
