#pragma once

#include "window.h"

namespace engine {
  class PalRenderer;
}

namespace editor {

class GamePanel : public Window {
 public:
  GamePanel(engine::PalRenderer* renderer, int width, int height, const std::string& title);
  ~GamePanel();
  /**
   * @brief 渲染逻辑
   *
   */
  virtual void render() override;

  virtual bool init() override;
private:
  engine::PalRenderer* _renderer = nullptr;
};
}  // namespace editor
