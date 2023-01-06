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

#pragma once

#include "base/Ptr.h"
#include "base/std/optional.h"
#include "core/assets/Asset.h"
#include "core/assets/AssetEnum.h"
#include "core/assets/TextureBase.h"

namespace cc {
/**
 * @en The create information for [[VideoTexture]]
 * @zh 创建视频贴图的信息。
 */
struct IVideoTextureCreateInfo {
    /**
     * @en The pixel width
     * @zh 像素宽度。
     */
    uint32_t width{0};

    /**
     * @en The pixel height
     * @zh 像素高度。
     */
    uint32_t height{0};

    /**
     * @en External resource handle. GL_ID or CVPixelBuffer.
     * @zh 外部资源句柄，可是 GL_ID 或 CVPixelBuffer
     */
    void* externalRes{nullptr};
};

class VideoTexture final : public TextureBase {
public:
    VideoTexture() = default;
    ~VideoTexture() override = default;

    void initialize(const IVideoTextureCreateInfo &info);

    gfx::Texture *getGFXTexture() const override;

private:
    IntrusivePtr<gfx::Texture> _gfxTexture;
};

} // namespace cc
