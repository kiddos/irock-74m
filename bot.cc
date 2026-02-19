#include <absl/strings/str_split.h>
#include <cpr/cpr.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <unistd.h>
#include <CLI/CLI.hpp>

#include <chrono>
#include <deque>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

#include "irock_74m.h"

using u32 = unsigned int;
using u64 = unsigned long long;

bool starts_with(const std::string& s, const std::string& p) {
  return s.substr(0, p.length()) == p;
}

std::optional<std::string> FindKeyboardDevice() {
  std::ifstream device_file("/proc/bus/input/devices", std::ifstream::in);
  if (device_file.is_open()) {
    std::string line;
    std::string handler;
    while (std::getline(device_file, line)) {
      if (line.empty()) {
        continue;
      }

      const std::string header = line.substr(0, 2);
      const std::string values = line.substr(3);
      if (header == "H:") {
        // handlers
        auto idx = values.find("=");
        if (idx != std::string::npos) {
          const std::string handlers_str = values.substr(idx);
          const std::vector<std::string> handlers =
              absl::StrSplit(handlers_str, " ");

          auto it = find(handlers.begin(), handlers.end(), "kbd");
          if (it != handlers.end()) {
            for (const std::string& h : handlers) {
              if (starts_with(h, "event")) {
                handler = h;
              }
            }
          }
        }
      } else if (header == "B:") {
        auto idx = values.find("=");
        if (idx != std::string::npos) {
          const std::string key = values.substr(0, idx);
          const std::string value = values.substr(idx + 1);
          if (key == "EV" && value == "120013") {
            if (!handler.empty()) {
              return std::optional("/dev/input/" + handler);
            }
          }
        }
      }
    }
  }
  return std::optional<std::string>();
}

int fd = -1;

void sigint_handler(int) {
  if (fd) {
    close(fd);
  }
};

u64 get_system_millis() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

struct KeyEvent {
  u32 keycode;
  u64 t;
};

void SetConfig(irock::Configuration config) {
  nlohmann::json body;
  body["mode"] = (int)config.mode;
  body["brightness"] = (int)config.brightness;
  body["speed"] = (int)config.speed;

  std::string json_data = body.dump();
  std::cout << "json=" << json_data << std::endl;
  cpr::Response r = cpr::Post(
      cpr::Url{"http://localhost:9100"},  // Replace with your target URL
      cpr::Header{{"Content-Type", "application/json"}}, cpr::Body{json_data});
}

int main(int argc, char* argv[]) {
  CLI::App app{"irock 74m bot"};
  argv = app.ensure_utf8(argv);

  int interval = 60 * 1000 * 5; // default 5 minute
  int idle_time = 60 * 1000 * 10; // 10 min
  app.add_option("--interval", interval, "");
  CLI11_PARSE(app, argc, argv);

  auto keyboard = FindKeyboardDevice();
  if (!keyboard) {
    std::cerr << "fail to find keyboard device" << std::endl;
    return 0;
  }

  std::cout << "device=" << *keyboard << std::endl;
  fd = open(keyboard->c_str(), O_RDONLY);
  if (fd < 0) {
    return 0;
  }

  signal(SIGINT, sigint_handler);

  std::deque<KeyEvent> d;

  struct input_event ev;
  u64 idle_start = 0;
  while (true) {
    u64 current_t = get_system_millis();
    size_t size = d.size();
    while (!d.empty() && current_t - d.front().t >= (size_t)interval) {
      d.pop_front();
    }
    if (size > 0 && d.empty()) {
      irock::Configuration config{
          irock::BREATHING,
          irock::BRIGHTNESS_5,
          irock::SPEED_1,
      };
      SetConfig(config);
      idle_start = current_t;
    } else if (idle_start > 0 && current_t - idle_start >= (size_t)idle_time) {
      irock::Configuration config{
          irock::OFF,
          irock::BRIGHTNESS_0,
          irock::SPEED_0,
      };
      SetConfig(config);
      idle_start = 0;
    }

    fd_set read_fds;
    struct timeval tv;

    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    int retval = select(fd+ 1, &read_fds, NULL, NULL, &tv);
    if (retval == -1) {
      // Error occurred
      if (errno == EINTR) {
        // Interrupted by a signal (e.g., Ctrl+C), continue
        continue;
      }
    } else if (retval == 0) {
      // Timeout occurred (no event within 1 second)
      continue;  // Go back to select()
    }

    ssize_t bytes_read = read(fd, &ev, sizeof(ev));
    if (bytes_read == sizeof(ev)) {
      if (ev.type == EV_KEY) {
        // ev.value: 1 = Key Press, 0 = Key Release, 2 = Key Repeat
        if (ev.value == 1) {
          u32 keycode = ev.code;
          u64 t = get_system_millis();
          if (d.empty()) {
            irock::Configuration config{
                irock::SHADOW_DISAPPEAR,
                irock::BRIGHTNESS_3,
                irock::SPEED_2,
            };
            SetConfig(config);
          }

          idle_start = 0;
          d.push_back(KeyEvent{keycode, t});
        }
      }
    } else if (bytes_read == -1) {
      break;
    }
  }

  close(fd);
  return 0;
}
