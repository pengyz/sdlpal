#pragma once

#include "window.h"
#include "engine/pal_log.h"
#include <cstdarg>

namespace engine {
class PalEngine;
}

namespace editor {

class LogPanel : public Window {
public:
    LogPanel(Window* parent, int width, int height, const std::string& title, bool visible, engine::PalEngine* engine);
    ~LogPanel();
    /**
     * @brief 渲染逻辑
     *
     */
    virtual void render() override;

    virtual bool init() override;

    void ClearLog();

    void AddLogAp(engine::LogLevel level, const char* fmt, va_list ap);

    void AddLog(engine::LogLevel level, const char* fmt, ...) IM_FMTARGS(3);

    void AddLog(const char* fmt, ...) IM_FMTARGS(2);

    void ExecCommand(const char* command_line);

    int TextEditCallback(ImGuiInputTextCallbackData* data);

private:
    engine::PalEngine* _engine = nullptr;
    char InputBuf[256];
    ImVector<const char*> Commands;
    ImVector<char*> History;
    int HistoryPos; // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter Filter;
    bool AutoScroll;
    bool ScrollToBottom;
};
} // namespace editor
