import { Hexagon } from "lucide-react";

export const HeaderLogo = () => {
  return (
    <div className="flex items-center gap-3">
      <Hexagon className="h-7 w-7 text-primary" />
      <h1 className="text-xl font-bold tracking-wider text-foreground hidden sm:block">
        Gen3D
      </h1>
    </div>
  );
};

export default HeaderLogo;
