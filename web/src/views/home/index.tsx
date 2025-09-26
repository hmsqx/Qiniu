import { GeneratorTabs } from "./_modules/generatorTabs";
import HomePage from "./_modules/homePage";
import { useCallback, useState } from "react";

const Home = () => {
  // Use callback ref + state so child receives an updated element and re-renders
  const [scrollRootEl, setScrollRootEl] = useState<HTMLDivElement | null>(null);
  const setScrollRef = useCallback((el: HTMLDivElement | null) => {
    setScrollRootEl(el);
  }, []);
  return (
    // Use fixed viewport height and hide outer overflow so only the right pane scrolls
    <div className="flex gap-2 h-screen overflow-hidden ">
      {/* Left: static area, no scroll */}
      <div className="flex-1 h-full overflow-hidden">
        <GeneratorTabs />
      </div>
      {/* Right: content area with its own vertical scroll */}
      <div
        ref={setScrollRef}
        className="flex-4 h-full overflow-y-auto min-h-0 pb-4 overscroll-contain"
      >
        <HomePage scrollRoot={scrollRootEl} />
      </div>
    </div>
  );
};

export default Home;
