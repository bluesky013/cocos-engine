/****************************************************************************
 Copyright (c) 2023-2023 Xiamen Yaji Software Co., Ltd.

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

#include "base/std/container/map.h"
#include "gfx-base/GFXDevice.h"
#include "core/assets/Material.h"
#include "scene/Pass.h"
#include "GPUTimeQuery.h"

#if CC_USE_DEBUG_RENDERER
#include "profiler/DebugRenderer.h"
#endif

namespace cc::render {
struct NativePipeline;

class PipelineProfiler {
public:
    PipelineProfiler();
    ~PipelineProfiler() = default;

    void beginFrame(uint32_t passCount, gfx::CommandBuffer *cmdBuffer);
    void endFrame(gfx::CommandBuffer *cmdBuffer);
    void resolveData(NativePipeline &pipeline);

    void writeGpuTimeStamp(gfx::CommandBuffer *cmdBuffer, uint32_t passID);
    void render(gfx::RenderPass *renderPass, uint32_t subPassID, gfx::CommandBuffer *cmdBuff);

private:
    void initMaterial();

    struct PassTime {
        uint64_t begin;
        uint64_t end;
    };

    gfx::Device *_device = nullptr;
    GPUTimeQuery _timeQuery;
    ccstd::map<uint32_t, PassTime> _passTimes;
    uint64_t _min = 0;
    uint64_t _max = 0;
    uint32_t _timestampPeriod = 1;
#if CC_USE_DEBUG_RENDERER
    std::unique_ptr<TextRenderer> _textRenderer;
    IntrusivePtr<Material> _material;
    IntrusivePtr<scene::Pass> _scenePass;
    float _width = 1.F;
    float _height = 1.F;
#endif
};

} // namespace cc::render
