//
// Created by Zach Lee on 2023/3/8.
//

#pragma once

#include "platform/interfaces/modules/ISystemWindow.h"

namespace cc {

class DummySystemWindow : public ISystemWindow {
public:
    explicit DummySystemWindow(uint32_t windowId, void* externalHandle);
    ~DummySystemWindow() override = default;

    bool createWindow(const char* title,
                      int x, int y, int w,
                      int h, int flags) override;
    bool createWindow(const char* title,
                      int w, int h, int flags) override;
    void closeWindow() override;

    uintptr_t getWindowHandle() const override { return _windowHandle; }
    uint32_t getWindowId() const override { return _windowId; }

    Size getViewSize() const override {
        return Size{static_cast<float>(_width), static_cast<float>(_height)};
    }
    void setViewSize(uint32_t w, uint32_t h) override {
        _width = w;
        _height = h;
    }

    /*
     @brief enable/disable(lock) the cursor, default is enabled
     */
    void setCursorEnabled(bool value) override {}
    void copyTextToClipboard(const std::string& text) override {}

    bool isDummyWindow() const override { return true; }
private:
    int32_t _width{0};
    int32_t _height{0};

    uint32_t _windowId{0};
    uintptr_t _windowHandle{0};
};

} // namespace cc
