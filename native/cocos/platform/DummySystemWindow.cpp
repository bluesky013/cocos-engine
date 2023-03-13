//
// Created by Zach Lee on 2023/3/8.
//

#include "DummySystemWindow.h"

namespace cc {
DummySystemWindow::DummySystemWindow(uint32_t windowId, void* externalHandle)
    : _windowId(windowId) {
    if (externalHandle) {
        _windowHandle = reinterpret_cast<uintptr_t>(externalHandle);
    }
}

bool DummySystemWindow::createWindow(const char* title,
                  int x, int y, int w,
                  int h, int flags)
{
    _width  = w;
    _height = h;
    return true;
}

bool DummySystemWindow::createWindow(const char* title,
                  int w, int h, int flags)
{
    return createWindow(title, 0, 0, w, h, flags);
}

void DummySystemWindow::closeWindow()
{
    // do nothing
}

} // namespace cc
