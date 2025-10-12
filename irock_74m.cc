#include "irock_74m.h"
#include <absl/log/log.h>
#include <iostream>

namespace irock {

Keyboard74M::Keyboard74M() : handle_(nullptr) {
  hid_device_info* info = hid_enumerate(VENDOR_ID, PRODUCT_ID);
  for (hid_device_info* cur = info; cur; cur = cur->next) {
    LOG(INFO) << L"Path: " << cur->path;
    LOG(INFO) << L"Manufacturer: " << (cur->manufacturer_string ? cur->manufacturer_string : L"(none)");
    LOG(INFO) << L"Product: " << (cur->product_string ? cur->product_string : L"(none)");
    LOG(INFO) << L"Serial: " << (cur->serial_number ? cur->serial_number : L"(none)");
    LOG(INFO) << L"Release number: " << std::hex << cur->release_number;
    LOG(INFO) << L"Interface number: " << cur->interface_number;

    if (cur->interface_number == INTERFACE) {
      LOG(INFO) << "found interface " << INTERFACE;
      handle_ = hid_open_path(cur->path);
    }
  }

  hid_free_enumeration(info);
}

void Keyboard74M::Close() {
  if (handle_) {
    hid_close(handle_);
  }
}

bool Keyboard74M::SetConfiguration(Mode mode, Brightness b, Speed s) {
  uint8_t message[] = {
    0x0a,0x01,0x01,0x02,0x29,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0
  };
  message[MODE_OFFSET] = mode;
  message[BRIGHTNESS_OFFSET] = b;
  message[SPEED_OFFSET] = s;

  int res = hid_send_feature_report(handle_, message, sizeof(message));
  if (res < 0) {
    LOG(INFO) << L"hid_write failed: " << hid_error(handle_);
    return false;
  }
  return true;
}

} // end of irock namespace
