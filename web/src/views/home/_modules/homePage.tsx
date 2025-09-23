import { FloatingImageZoom } from "./FloatingImageZoom";
import { InspirationCard } from "./InspirationCard";
import { SectionHeader } from "./SectionHeader";
import { inspirations } from "../data/inspirations";

const heroImage = "/homePage.jpg";

export default function HomePage() {
  return (
    <main className="min-h-screen ">
      <div className="mx-auto">
        <div className="flex px-2 gap-2 ">
          <FloatingImageZoom
            src={heroImage}
            className="h-[260px] md:h-[360px]"
          />
          <FloatingImageZoom
            src={heroImage}
            className="hidden h-[260px] md:block md:h-[360px]"
          />
        </div>

        <section className="mt-8 lg:mt-12">
          <SectionHeader title="灵感广场" />
          <div className="grid grid-cols-2 gap-x-4 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 xl:grid-cols-5 2xl:grid-cols-6">
            {inspirations.map((item, idx) => (
              <InspirationCard key={idx} item={item} />
            ))}
          </div>
        </section>
      </div>
    </main>
  );
}
