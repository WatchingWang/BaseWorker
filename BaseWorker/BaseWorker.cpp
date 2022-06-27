#include "BaseWorker.h"

#include <future>

int BaseWorker::Start() {
  if (is_started_ == true) {
    Stop(CANCEL_LEFT_FUNC);
  }

  is_started_.exchange(true);

  thread_ = std::make_unique<std::thread>([=] {
    while (is_started_) {
      std::unique_lock<decltype(mtx_)> lck(mtx_);
      if (func_queue_.empty()) {
        cv_.wait(lck, [=] { return (!is_started_ || !func_queue_.empty()); });
      }
      if (!is_started_) {
        return;
      }

      auto func = func_queue_.front();
      func_queue_.pop();
      lck.unlock();

      (*func)();
    }
  });

  return 0;
}

int BaseWorker::Stop(StopOption option) {
  if (is_started_ == false) {
    return 0;
  }

  is_stopping_.exchange(true);

  if (option == CANCEL_LEFT_FUNC) {
    std::unique_lock<decltype(mtx_)> lck(mtx_);
    is_started_.exchange(false);
    while (!func_queue_.empty()) {
      func_queue_.pop();
    }

    cv_.notify_all();
  } else if (option == SYNC_FINISH_LEFT_FUNC) {
    std::unique_lock<decltype(mtx_)> lck(mtx_);

    cv_.wait(lck, [=] { return !func_queue_.empty(); });
    is_started_.exchange(false);
    cv_.notify_all();
  } else {
    return -1;
  }

  if (thread_->joinable()) {
    thread_->join();
  }

  is_stopping_.exchange(false);

  return 0;
}

void BaseWorker::AsyncCall(const std::function<void(void)>& func) {
  BaseWorkerFunc temp_func =
      std::make_shared<std::function<void(void)>>([&] { func(); });
  {
    std::lock_guard<decltype(mtx_)> lck(mtx_);
    if (!is_stopping_) {
      func_queue_.emplace(std::move(temp_func));
      cv_.notify_all();
    }
  }
}

int BaseWorker::SyncCall(const std::function<int(void)>& func) {
  std::promise<int> promise;
  auto fut = promise.get_future();

  int ret = 0;
  std::function<void(void)> temp_func([&] {
    ret = func();
    promise.set_value(ret);
  });

  AsyncCall(temp_func);

  return fut.get();
}
