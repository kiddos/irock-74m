#include <absl/log/log.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/util.h>
#include <hidapi/hidapi.h>
#include <signal.h>

#include <memory>
#include <nlohmann/json.hpp>

#include "irock_74m.h"

static std::unique_ptr<irock::Keyboard74M> keyboard;

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

void request_handler(evhttp_request* req, void*);
void signal_cb(evutil_socket_t sig, short, void* arg);

int main() {
  if (hid_init() != 0) {
    LOG(ERROR) << "Failed to initialize hidapi";
    return 1;
  }

  event_base* base = event_base_new();
  if (!base) {
    LOG(ERROR) << "fail to create event_base";
    return 1;
  }
  evhttp* http = evhttp_new(base);
  if (!http) {
    LOG(ERROR) << "fail to create evhttp";
    return 1;
  }

  event* signal_event =
      event_new(base, SIGINT, EV_SIGNAL | EV_PERSIST, signal_cb, (void*)base);
  if (!signal_event) {
    LOG(ERROR) << "fail to create signal event";
    evhttp_free(http);
    event_base_free(base);
    return 1;
  }

  if (event_add(signal_event, nullptr) < 0) {
    LOG(ERROR) << "fail to add signal event";
    event_free(signal_event);
    event_base_free(base);
    return 1;
  }

  using namespace irock;
  keyboard = std::make_unique<irock::Keyboard74M>();

  evhttp_set_gencb(http, request_handler, nullptr);

  // Bind the HTTP server to an address and port
  unsigned short port = 9100;
  std::string http_addr = "0.0.0.0";
  evhttp_bound_socket* handle = evhttp_bind_socket_with_handle(http, http_addr.c_str(), port);
  if (!handle) {
    LOG(ERROR) << "fail to bind address: " << http_addr << ":" << port;
    return 1;
  }

  LOG(INFO) << "Libevent HTTP server listening on http://" << http_addr << ":"
            << port;
  LOG(INFO) << "Press Ctrl+C to stop.";

  event_base_dispatch(base);

  LOG(INFO) << "closing keyboard hid";
  keyboard->Close();
  hid_exit();

  evhttp_free(http);
  event_base_free(base);
  return 0;
}

void signal_cb(evutil_socket_t sig, short, void* arg) {
  event_base* base = (event_base*)arg;
  if (base) {
    LOG(INFO) << "Caught signal " << sig << " stopping the event loop...";
    event_base_loopbreak(base);
  }
}

void request_handler(evhttp_request* req, void*) {
  if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
    evhttp_send_error(req, HTTP_BADREQUEST, "Only POST supported");
    return;
  }

  struct evbuffer* input_buf = evhttp_request_get_input_buffer(req);
  size_t post_data_len = evbuffer_get_length(input_buf);
  char* post_data = (char*)malloc(post_data_len + 1);
  if (!post_data) {
    return;
  }

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
