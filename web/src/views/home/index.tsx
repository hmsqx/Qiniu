import { GeneratorTabs } from "./_modules/generatorTabs";
import HomePage from "./_modules/homePage";
import { useCallback, useState } from "react";

const Home = () => {
  const [scrollRootEl, setScrollRootEl] = useState<HTMLDivElement | null>(null);
  const setScrollRef = useCallback((el: HTMLDivElement | null) => {
    setScrollRootEl(el);
  }, []);
  return (
    <div className="flex gap-2 h-full overflow-hidden pt-2">
      <div className="flex-1 h-full overflow-hidden">
        <GeneratorTabs />
      </div>
      <div
        ref={setScrollRef}
        className="flex-4 h-full overflow-y-auto min-h-0 pb-4 overscroll-contain "
      >
        <HomePage scrollRoot={scrollRootEl} />
      </div>
    </div>
  );
};

export default Home;
