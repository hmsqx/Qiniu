import { MenuBar, type MenuItem } from "../menu";
import { preloadMap } from "./constants";

type Props = {
  items: MenuItem[];
  activeItem: string;
  onItemClick: (label: string) => void;
};

export const HeaderMenu = ({ items, activeItem, onItemClick }: Props) => {
  return (
    <div className="hidden md:block">
      <MenuBar
        items={items}
        activeItem={activeItem}
        onItemClick={onItemClick}
        onItemHover={(label) => preloadMap[label]?.()}
      />
    </div>
  );
};

export default HeaderMenu;
