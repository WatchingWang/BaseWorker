#include <atomic>
#include <functional>
#include <queue>
#include <thread>

class BaseWorker {
 public:
  using BaseWorkerFunc = std::shared_ptr<std::function<void(void)>>;
  enum StopOption {
    CANCEL_LEFT_FUNC,
    SYNC_FINISH_LEFT_FUNC,
  };

 public:
  int Start();
  int Stop(StopOption option = CANCEL_LEFT_FUNC);
  int SyncCall(const std::function<int(void)>& func);
  void AsyncCall(const std::function<void(void)>& func);

 private:
  std::atomic_bool is_started_ = {false};
  std::atomic_bool is_stopping_ = {false};
  std::unique_ptr<std::thread> thread_;

  std::queue<BaseWorkerFunc> func_queue_;
  std::mutex mtx_;
  std::condition_variable cv_;
};