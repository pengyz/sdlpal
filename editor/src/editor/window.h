#pragma once
#include "imgui.h"
#include <functional>
#include <map>
#include <string>

namespace editor {

class Window {
public:
    Window(Window* parent, int width, int height, const std::string& title, bool visible = true)
        : _parent(parent)
        , _width(width)
        , _height(height)
        , _title(title)
        , _visible(visible)
    {
    }
    virtual ~Window() = default;

public:
    virtual bool init() = 0;
    virtual void render() = 0;
    bool getVisible() { return _visible; }
    void setVisible(bool visible) { _visible = visible; }


protected:
    Window* _parent = nullptr;
    int _width = 0;
    int _height = 0;
    bool _visible = true;
    std::string _title;
};
} // namespace editor
