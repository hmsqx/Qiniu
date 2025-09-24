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
import React from "react";
import { toggleJobVisibility, type JobItem } from "@/api/mode3D";
import { useToast } from "@/components/ui/use-toast";

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

  const [selectionMode, setSelectionMode] = React.useState(false);
  const [selectedIds, setSelectedIds] = React.useState<string[]>([]);
  const [bulkLoading, setBulkLoading] = React.useState(false);
  const [togglingIds, setTogglingIds] = React.useState<Set<string>>(new Set());
  const { toast } = useToast();

  React.useEffect(() => {
    if (!selectionMode) setSelectedIds([]);
  }, [selectionMode]);

  function toggleSelect(jobId: string) {
    setSelectedIds((prev) =>
      prev.includes(jobId)
        ? prev.filter((id) => id !== jobId)
        : [...prev, jobId]
    );
  }

  async function bulkToggle(target: "public" | "private") {
    if (selectedIds.length === 0) return;
    const idsToProcess = [...selectedIds];
    setSelectedIds([]);
    setSelectionMode(false);
    setBulkLoading(true);
    let successCount = 0;
    let failCount = 0;
    const toastId = toast({
      title: target === "public" ? "正在批量设为公开" : "正在批量设为私有",
      description: `共 ${idsToProcess.length} 个任务...`,
      variant: "loading",
    });
    try {
      await Promise.all(
        idsToProcess.map(async (id) => {
          try {
            await toggleJobVisibility(id);
            successCount++;
          } catch (e) {
            failCount++;
          }
          const processed = successCount + failCount;
          if (processed === idsToProcess.length || processed % 5 === 0) {
            toast({
              id: toastId,
              title:
                target === "public" ? "正在批量设为公开" : "正在批量设为私有",
              description: `已处理 ${processed} / ${idsToProcess.length} 个...`,
              variant: "loading",
            });
          }
        })
      );
      await refresh();
      const totalCount = idsToProcess.length;
      if (failCount === 0) {
        toast({
          id: toastId,
          title: target === "public" ? "批量设为公开完成" : "批量设为私有完成",
          description: `成功处理 ${successCount} 个任务`,
          variant: "success",
        });
      } else if (successCount === 0) {
        toast({
          id: toastId,
          title: "批量操作失败",
          description: `全部 ${failCount} 个任务失败`,
          variant: "error",
        });
      } else {
        toast({
          id: toastId,
          title: "部分成功",
          description: `成功 ${successCount} / 失败 ${failCount} (共 ${totalCount})`,
          variant: "error",
        });
      }
    } finally {
      setBulkLoading(false);
    }
  }

  const handleBulkPublic = () => bulkToggle("public");
  const handleBulkPrivate = () => bulkToggle("private");

  async function handleSingleToggle(job: JobItem) {
    if (!job?.jobId) return;
    setTogglingIds((prev) => new Set(prev).add(job.jobId));
    const id = toast({
      title: "正在切换...",
      description: `任务 ${job.jobId}`,
      variant: "loading",
    });
    try {
      await toggleJobVisibility(job.jobId);
      refresh();
      setSelectedIds((prev) => prev.filter((id) => id !== job.jobId));
      // 更新成成功
      toast({
        id,
        title: job.isPrivate ? "已设为公开" : "已设为私有",
        description: `任务 ${job.jobId}`,
        variant: "success",
      });
    } catch (e) {
      console.warn("single toggle failed", job.jobId, e);
      toast({
        id,
        title: "切换失败",
        description: `任务 ${job.jobId}`,
        variant: "error",
      });
    } finally {
      setTogglingIds((prev) => {
        const copy = new Set(prev);
        copy.delete(job.jobId);
        return copy;
      });
    }
  }

  const totalCountCurrentPage = list.length;

  function enterSelectionMode() {
    if (!selectionMode) {
      setSelectionMode(true);
      setSelectedIds([]);
    }
  }

  function exitSelectionMode() {
    if (selectionMode) {
      setSelectionMode(false);
      setSelectedIds([]);
    }
  }

  function toggleAllOnPage() {
    if (!selectionMode) return;
    if (selectedIds.length === totalCountCurrentPage) {
      setSelectedIds([]);
    } else {
      setSelectedIds(list.map((it) => it.jobId));
    }
  }

  // ESC 键退出多选
  React.useEffect(() => {
    if (!selectionMode) return;
    function onKey(e: KeyboardEvent) {
      if (e.key === "Escape") {
        exitSelectionMode();
      }
    }
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [selectionMode]);

  return (
    <div className="p-4 sm:p-6  text-white min-h-screen overflow-y-auto no-scrollbar">
      <Toolbar
        loading={loading || bulkLoading}
        onRefresh={refresh}
        selectionMode={selectionMode}
        onEnterSelectionMode={enterSelectionMode}
        onExitSelectionMode={exitSelectionMode}
        onToggleAll={toggleAllOnPage}
        allSelected={
          selectionMode &&
          totalCountCurrentPage > 0 &&
          selectedIds.length === totalCountCurrentPage
        }
        selectedCount={selectedIds.length}
        totalCount={totalCountCurrentPage}
        onBulkPublic={handleBulkPublic}
        onBulkPrivate={handleBulkPrivate}
        disableBulkAction={bulkLoading}
      />

      <div className="mt-8">
        {error && <ErrorState message={error} />}

        <Grid>
          {loading
            ? Array.from({ length: Math.max(4, Math.min(6, pageSize)) }).map(
                (_, idx) => <SkeletonCard key={idx} />
              )
            : list.length > 0
            ? list.map((it) => (
                <JobCard
                  key={it.jobId}
                  item={it}
                  selectable={selectionMode}
                  selected={selectedIds.includes(it.jobId)}
                  onToggleSelect={toggleSelect}
                  onToggleVisibility={handleSingleToggle}
                  toggling={togglingIds.has(it.jobId)}
                />
              ))
            : !error && <EmptyState />}
        </Grid>
      </div>

      {!error && (
        <Pagination
          total={total}
          pageNum={pageNum}
          totalPages={totalPages}
          loading={loading || bulkLoading}
          onPrev={prevPage}
          onNext={nextPage}
        />
      )}
    </div>
  );
}
