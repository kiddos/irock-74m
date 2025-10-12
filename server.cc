#include <absl/log/log.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/util.h>
#include <hidapi/hidapi.h>
#include <signal.h>

#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

#include "irock_74m.h"

static std::unique_ptr<irock::Keyboard74M> keyboard;

void sigint_handler(int) {
  if (keyboard) {
    keyboard->Close();
  }
  hid_exit();
  exit(0);
}

std::optional<irock::Configuration> parse(nlohmann::json& value) {
  std::optional<irock::Configuration> c;
  std::vector<std::string> check_int = {"mode", "brightness", "speed"};
  for (std::string& field : check_int) {
    if (!value[field].is_number_integer()) {
      LOG(WARNING) << "invalid type for " << field << " expect integer";
      return c;
    }
  }

  irock::Configuration conf;
  conf.mode = (irock::Mode)value["mode"].get<int>();
  conf.brightness = (irock::Brightness)value["brightness"].get<int>();
  conf.speed = (irock::Speed)value["speed"].get<int>();
  return std::optional(conf);
}

class HTTPServer {
 public:
  HTTPServer() : base_(nullptr), http_(nullptr), handle_(nullptr) { init(); }
  ~HTTPServer() {
    if (http_) {
      evhttp_free(http_);
    }

    if (base_) {
      event_base_free(base_);
    }
  }

  void init() {
    base_ = event_base_new();
    if (!base_) {
      return;
    }

    http_ = evhttp_new(base_);
    if (!http_) {
      return;
    }
  }

  void Start() {
    LOG(INFO) << "starting http server...";
    evhttp_set_gencb(http_, request_handler, nullptr);

    // Bind the HTTP server to an address and port
    unsigned short port = 9100;
    std::string http_addr = "0.0.0.0";
    handle_ = evhttp_bind_socket_with_handle(http_, http_addr.c_str(), port);
    if (!handle_) {
      std::cerr << "fail to bind address: " << http_addr << ":" << port
                << std::endl;
      return;
    }
    LOG(INFO) << "Libevent HTTP server listening on http://" << http_addr << ":"
              << port;
    LOG(INFO) << " Press Ctrl+C to stop.";

    event_base_dispatch(base_);
  }

  static void request_handler(evhttp_request* req, void*) {
    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
      evhttp_send_error(req, HTTP_BADREQUEST, "Only POST supported");
      return;
    }

    struct evbuffer* input_buf = evhttp_request_get_input_buffer(req);
    size_t post_data_len = evbuffer_get_length(input_buf);
    char* post_data = (char*)malloc(post_data_len + 1);
    if (!post_data) return;

    evbuffer_copyout(input_buf, post_data, post_data_len);
    post_data[post_data_len] = '\0';

    std::string content = post_data;
    free(post_data);

    auto json = nlohmann::json::parse(content);
    auto config = parse(json);
    if (config && keyboard) {
      LOG(INFO) << "set config";
      keyboard->SetConfiguration(config->mode, config->brightness,
                                 config->speed);
    }

    evhttp_send_reply(req, 200, "OK", NULL);
  }

 private:
  event_base* base_;
  evhttp* http_;
  evhttp_bound_socket* handle_;
};

int main() {
  if (hid_init() != 0) {
    std::cerr << "Failed to initialize hidapi\n";
    return 1;
  }
  signal(SIGINT, &sigint_handler);

  using namespace irock;
  keyboard = std::make_unique<irock::Keyboard74M>();
  keyboard->SetConfiguration(STEADY, BRIGHTNESS_1, SPEED_0);

  HTTPServer server;
  server.Start();

  keyboard->Close();
  hid_exit();
  return 0;
}
