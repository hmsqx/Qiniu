import { useAuth } from "@/context/AuthContext";
import { useWorkspaceJobs } from "./workspace-gallery/hooks/useWorkspaceJobs";
import { Toolbar } from "./workspace-gallery/components/Toolbar";
import {
  Grid,
  EmptyState,
  ErrorState,
} from "./workspace-gallery/components/Grid";
import { SkeletonCard } from "./workspace-gallery/components/SkeletonCard";
import { JobCard } from "./workspace-gallery/components/JobCard";
import { Pagination } from "./workspace-gallery/components/Pagination";

export default function WorkspaceGallery() {
  const { user } = useAuth();
  const userId = user?.id || "";

  const {
    list,
    total,
    pageNum,
    pageSize,
    totalPages,
    loading,
    error,
    refresh,
    nextPage,
    prevPage,
  } = useWorkspaceJobs(userId, { initialPage: 1, pageSize: 10 });

  return (
    <div className="p-4 sm:p-6  text-white min-h-screen overflow-y-auto no-scrollbar">
      <Toolbar loading={loading} onRefresh={refresh} />

      <div className="mt-8">
        {error && <ErrorState message={error} />}

        <Grid>
          {loading
            ? Array.from({ length: Math.max(4, Math.min(6, pageSize)) }).map(
                (_, idx) => <SkeletonCard key={idx} />
              )
            : list.length > 0
            ? list.map((it) => <JobCard key={it.jobId} item={it} />)
            : !error && <EmptyState />}
        </Grid>
      </div>

      {!error && (
        <Pagination
          total={total}
          pageNum={pageNum}
          totalPages={totalPages}
          loading={loading}
          onPrev={prevPage}
          onNext={nextPage}
        />
      )}
    </div>
  );
}
