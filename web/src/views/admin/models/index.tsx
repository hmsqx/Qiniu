import React from "react";
import { ModelFilters, type ModelFiltersState } from "./_modules/ModelFilters";
import { ModelsTable } from "./_modules/ModelsTable";
import { Pagination } from "@/views/workspace/_modules/Pagination";
import { useModels } from "./hooks/useModels";

const AdminModels: React.FC = () => {
  const {
    list,
    total,
    page,
    pageSize,
    loading,
    nextPage,
    prevPage,
    setPage,
    minLike,
    setMinLike,
    maxLike,
    setMaxLike,
    minDownload,
    setMinDownload,
    maxDownload,
    setMaxDownload,
    refresh,
  } = useModels();
  const totalPages = Math.max(1, Math.ceil(total / pageSize));

  const applyFilters = (s: ModelFiltersState) => {
    setMinLike(typeof s.minLike === "number" ? s.minLike : undefined);
    setMaxLike(typeof s.maxLike === "number" ? s.maxLike : undefined);
    setMinDownload(
      typeof s.minDownload === "number" ? s.minDownload : undefined
    );
    setMaxDownload(
      typeof s.maxDownload === "number" ? s.maxDownload : undefined
    );
    setPage(1);
  };
  const refreshWithCurrent = () => {
    setPage(1);
    refresh();
  };

  const handlePrev = () => page > 1 && prevPage();
  const handleNext = () => page < totalPages && nextPage();
  const handleJump = (p: number) => setPage(p);

  return (
    <div className="mx-auto w-full max-w-[1600px] space-y-6 pb-10">
      <ModelFilters
        initial={{
          minLike,
          maxLike,
          minDownload,
          maxDownload,
        }}
        loading={loading}
        onApply={applyFilters}
        onRefresh={refreshWithCurrent}
      />

      <ModelsTable
        list={list}
        loading={loading}
        baseIndex={(page - 1) * pageSize}
      />

      <div className="mt-4">
        <Pagination
          total={total}
          pageNum={page}
          totalPages={totalPages}
          loading={loading}
          onPrev={handlePrev}
          onNext={handleNext}
          onJump={handleJump}
        />
      </div>
    </div>
  );
};

export default AdminModels;
