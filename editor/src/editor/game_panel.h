#pragma once

#include "window.h"

namespace engine {
  class PalRenderer;
}

namespace editor {

class  GamePanel : public Window {
 public:
  GamePanel(int width, int height, const std::string& title, engine::PalRenderer* renderer);
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
