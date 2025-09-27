import { cn } from "@/lib/utils";
import { Check } from "lucide-react";

export type PricingCardProps = {
  title: string;
  price: string;
  priceUnit?: string;
  description?: string;
  features: string[];
  popular?: boolean;
  selected?: boolean;
  onClick?: () => void;
  disabled?: boolean;
};

export const PricingCard: React.FC<PricingCardProps> = ({
  title,
  price,
  priceUnit,
  description,
  features,
  popular,
  selected,
  onClick,
  disabled,
}) => {
  return (
    <button
      disabled={disabled}
      onClick={onClick}
      className={cn(
        "group relative text-left w-full rounded-2xl p-[1px] transition",
        selected
          ? "bg-gradient-to-br from-primary/70 via-primary/20 to-transparent"
          : "bg-border",
        disabled && "opacity-60 cursor-not-allowed"
      )}
    >
      <div
        className={cn(
          "relative rounded-2xl border bg-card p-4 sm:p-5 h-full",
          "hover:shadow-[0_8px_24px_-12px_rgba(0,0,0,0.35)] hover:-translate-y-0.5 duration-200",
          selected ? "border-transparent" : "border-border"
        )}
      >
        {popular && (
          <span className="absolute right-3 top-3 rounded-full bg-primary text-primary-foreground px-2 py-0.5 text-[10px] tracking-wide shadow-sm">
            人气
          </span>
        )}
        <div className="flex items-baseline gap-2">
          <div className="text-xl font-semibold">{title}</div>
        </div>
        <div className="mt-1 flex items-end gap-1">
          <div className="text-3xl font-bold text-foreground">{price}</div>
          {priceUnit && (
            <div className="text-muted-foreground text-sm mb-1">{priceUnit}</div>
          )}
        </div>
        {description && (
          <div className="mt-1 text-sm text-muted-foreground">{description}</div>
        )}
        <ul className="mt-3 space-y-2">
          {features.map((f, i) => (
            <li key={i} className="flex items-center gap-2 text-sm">
              <Check className="h-4 w-4 text-primary" />
              <span>{f}</span>
            </li>
          ))}
        </ul>
      </div>
    </button>
  );
};

export default PricingCard;
