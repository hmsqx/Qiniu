import React from "react";

interface ErrorBoundaryState {
  hasError: boolean;
  error?: Error;
}

class ErrorBoundary extends React.Component<
  React.PropsWithChildren,
  ErrorBoundaryState
> {
  state: ErrorBoundaryState = { hasError: false };

  static getDerivedStateFromError(error: Error): ErrorBoundaryState {
    return { hasError: true, error };
  }

  componentDidCatch(error: Error, errorInfo: unknown) {
    console.error("ErrorBoundary caught", error, errorInfo);
  }

  handleRetry = () => {
    this.setState({ hasError: false, error: undefined });
  };

  render() {
    if (this.state.hasError) {
      return (
        <div className="p-6 space-y-4 text-sm">
          <h2 className="text-base font-semibold">模块加载失败</h2>
          <p className="text-muted-foreground">
            {this.state.error?.message || "未知错误"}
          </p>
          <button
            onClick={this.handleRetry}
            className="px-3 py-2 rounded bg-primary text-primary-foreground text-xs"
          >
            重试
          </button>
        </div>
      );
    }
    return this.props.children;
  }
}

export default ErrorBoundary;
