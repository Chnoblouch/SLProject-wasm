#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <Utils.h>
#include <CustomLog.h>
#include <imgui.h>

class LogWindow : public Utils::CustomLog
{
public:
    LogWindow(int screenWidth, int screenHeight);

    void post(const std::string& message) override;

    void clear();
    void draw(ImFont* fontText, ImFont* fontHeading, const char* title, bool* p_open = NULL);

private:
    void addLog(const char* fmt, ...) IM_FMTARGS(2);

    ImGuiTextBuffer _buf;
    ImGuiTextFilter _filter;
    ImVector<int>   _lineOffsets; // Index to lines offset. We maintain this with AddLog() calls, allowing us to have a random access on lines
    bool            _autoScroll;  // Keep scrolling if already at the bottom

    int _screenW;
    int _screenH;
};

#endif
