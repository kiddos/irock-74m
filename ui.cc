#include <GLFW/glfw3.h>
#include <CLI/CLI.hpp>
#include <absl/log/log.h>
#include <cpr/cpr.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include <vector>

#include "irock_74m.h"

struct ModeItem {
  int value;
  const char* name;
};

static const std::vector<ModeItem> kModes = {
    {irock::BREATHING, "Breathing (3)"},
    {irock::STEADY, "Steady (1)"},
    {irock::PRESS_AND_DESTROY, "Press and Destroy (4)"},
    {irock::NEON_STREAM, "Neon Stream (5)"},
    {irock::STREAMER, "Streamer (6)"},
    {irock::AMBILIGHT, "Ambilight (7)"},
    {irock::DRIPPING_RIPPLE, "Dripping Ripple (8)"},
    {irock::BRILLIANT_POINT, "Brilliant Point (9)"},
    {irock::FLASH_AWAY, "Flash Away (10)"},
    {irock::SHADOW_DISAPPEAR, "Shadow Disappear (11)"},
    {irock::RIPPLE_SHINING, "Ripple Shining (12)"},
    {irock::RICH_AND_HONOURED, "Rich and Honoured (13)"},
    {irock::MARQUEE_EFFECT, "Marquee Effect (14)"},
    {irock::ROTATING_STORM, "Rotating Storm (15)"},
    {irock::SERPENTINE_HORSE_RACE, "Serpentine Horse Race (16)"},
    {irock::STARS_TWINKLE, "Stars Twinkle (17)"},
    {irock::RETRO_SNAKE, "Retro Snake (18)"},
    {irock::DIAGNAL_TRANSFORMATION, "Diagonal Transformation (19)"},
    {irock::SINE_WAVE, "Sine Wave (20)"},
    {irock::OFF, "Off (21)"},
};

static void glfw_error_callback(int error, const char* description) {
  LOG(ERROR) << "GLFW Error " << error << ": " << description;
}

int main(int argc, char* argv[]) {
  CLI::App app{"irock 74m UI"};
  argv = app.ensure_utf8(argv);

  std::string server_url = "http://localhost:9100";
  app.add_option("--url", server_url, "Server URL");
  CLI11_PARSE(app, argc, argv);

  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    return 1;
  }

  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  GLFWwindow* window = glfwCreateWindow(500, 400, "irock 74M LED Controller", nullptr, nullptr);
  if (window == nullptr) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // State initialization
  int current_mode = 3;         // default mode: 3 (BREATHING)
  int current_brightness = 3;   // default brightness: 3
  int current_speed = 2;        // default speed: 2

  char url_buffer[256];
  snprintf(url_buffer, sizeof(url_buffer), "%s", server_url.c_str());

  std::string status_message = "Ready";
  bool is_pending = false;
  std::future<std::pair<long, std::string>> send_future;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Check async request
    if (is_pending && send_future.valid() &&
        send_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
      auto [status_code, body] = send_future.get();
      is_pending = false;
      if (status_code == 200) {
        status_message = "Success (200 OK)";
      } else {
        status_message = "Error (" + std::to_string(status_code) + "): " + body;
      }
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create main window UI
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    ImGui::SetNextWindowSize(ImVec2((float)display_w, (float)display_h), ImGuiCond_Always);

    ImGui::Begin("iRock 74M LED Controller", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    ImGui::Text("iRock 74M Keyboard Lighting Control");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::InputText("Server URL", url_buffer, sizeof(url_buffer));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Combo for Mode
    int current_mode_idx = 0;
    for (size_t i = 0; i < kModes.size(); ++i) {
      if (kModes[i].value == current_mode) {
        current_mode_idx = static_cast<int>(i);
        break;
      }
    }

    if (ImGui::BeginCombo("Mode", kModes[current_mode_idx].name)) {
      for (size_t i = 0; i < kModes.size(); ++i) {
        const bool is_selected = (current_mode_idx == static_cast<int>(i));
        if (ImGui::Selectable(kModes[i].name, is_selected)) {
          current_mode = kModes[i].value;
        }
        if (is_selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    // Slider for Brightness (0-5)
    ImGui::SliderInt("Brightness", &current_brightness, 0, 5);

    // Slider for Speed (0-5)
    ImGui::SliderInt("Speed", &current_speed, 0, 5);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Preview JSON
    nlohmann::json json_payload;
    json_payload["mode"] = current_mode;
    json_payload["brightness"] = current_brightness;
    json_payload["speed"] = current_speed;
    std::string json_str = json_payload.dump(4);

    ImGui::Text("Payload Preview:");
    ImGui::TextUnformatted(json_str.c_str());

    ImGui::Spacing();

    // Send Button
    if (is_pending) {
      ImGui::BeginDisabled();
    }
    if (ImGui::Button("Send Configuration", ImVec2(200, 30))) {
      status_message = "Sending request...";
      is_pending = true;

      std::string target_url(url_buffer);
      int mode_val = current_mode;
      int brightness_val = current_brightness;
      int speed_val = current_speed;

      send_future = std::async(std::launch::async, [target_url, mode_val, brightness_val, speed_val]() {
        nlohmann::json body;
        body["mode"] = mode_val;
        body["brightness"] = brightness_val;
        body["speed"] = speed_val;

        cpr::Response r = cpr::Post(
            cpr::Url{target_url},
            cpr::Header{{"Content-Type", "application/json"}},
            cpr::Body{body.dump()});

        return std::make_pair(r.status_code, r.text.empty() ? r.error.message : r.text);
      });
    }
    if (is_pending) {
      ImGui::EndDisabled();
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset Defaults", ImVec2(120, 30))) {
      current_mode = 3;
      current_brightness = 3;
      current_speed = 2;
    }

    ImGui::Spacing();
    ImGui::Text("Status: %s", status_message.c_str());

    ImGui::End();

    // Rendering
    ImGui::Render();
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
