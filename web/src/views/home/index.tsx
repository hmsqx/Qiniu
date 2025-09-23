import { GeneratorTabs } from "./_modules/generatorTabs";
import HomePage from "./_modules/homePage";
const Home = () => {
  return (
    <div className="flex gap-2 mt-2">
      <div className="h-screen flex-1">
        <GeneratorTabs />
      </div>
      <div className="h-screen flex-4">
        <HomePage />
      </div>
    </div>
  );
};

export default Home;
