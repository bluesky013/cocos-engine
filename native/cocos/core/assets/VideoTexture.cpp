/****************************************************************************
 Copyright (c) 2021-2023 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
****************************************************************************/

#include "core/assets/VideoTexture.h"
#include "gfx-base/GFXTexture.h"
#include "gfx-base/GFXDevice.h"

namespace cc {

void VideoTexture::initialize(const IVideoTextureCreateInfo &info) {
    gfx::TextureInfo texInfo = {};
    texInfo.usage  = gfx::TextureUsage::SAMPLED;
    texInfo.format = gfx::Format::RGBA8;
    texInfo.width  = info.width;
    texInfo.height = info.height;
    texInfo.externalRes = reinterpret_cast<void *>(info.externalRes);
    _gfxTexture = gfx::Device::getInstance()->createTexture(texInfo);
}

gfx::Texture *VideoTexture::getGFXTexture() const {
    return _gfxTexture;
}

} // namespace cc
