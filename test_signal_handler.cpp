#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

class SignalHandler
{
private:
  std::mutex m_sig_mtx;

  SignalHandler() = default;

public:
  SignalHandler(SignalHandler &other) = delete;
  SignalHandler(SignalHandler &&other) = delete;
  SignalHandler &operator=(SignalHandler &other) = delete;
  SignalHandler &operator=(SignalHandler &&other) = delete;

  static SignalHandler &
  instance() {
    static SignalHandler inst;
    return inst;
  }

  static void
  handle_signal([[maybe_unused]] int sig_num,
                [[maybe_unused]] siginfo_t *sig_inf,
                [[maybe_unused]] void *args) {
    using namespace std::chrono_literals;

    {
      std::scoped_lock sl{instance().m_sig_mtx};
      std::cout << "Thread " << std::this_thread::get_id() << " caught signal "
                << sig_num << " at address " << sig_inf->si_addr << std::endl;
    }

    volatile bool stop = false;
    while (!stop) {
      std::this_thread::sleep_for(1min);
    }
  }

  static void register_handler() {
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = SignalHandler::handle_signal;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &sa, nullptr);
  }
};

void
cause_sigsegv() {
  volatile int *ptr = nullptr;
  *ptr = 1;
}

int
main() {
  SignalHandler::register_handler();

  std::vector<std::thread> threads;
  for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
    threads.emplace_back(cause_sigsegv);
  }

  for (std::thread &t : threads) {
    t.join();
  }

  return 0;
}
