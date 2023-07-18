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

#include "PipelineProfiler.h"
#include "cocos/renderer/pipeline/custom/NativePipelineTypes.h"
#include "cocos/renderer/pipeline/custom/RenderGraphGraphs.h"
#include "cocos/profiler/DebugRenderer.h"

namespace cc::render {

void PipelineProfiler::beginFrame(uint32_t passCount, gfx::CommandBuffer *cmdBuffer) {
    timeQuery.resize(passCount * 2);
    timeQuery.reset(cmdBuffer);
    passTimes.clear();
}

void PipelineProfiler::endFrame(gfx::CommandBuffer *cmdBuffer) {
    timeQuery.copyResult(cmdBuffer);
}

void PipelineProfiler::writeGpuTimeStamp(gfx::CommandBuffer *cmdBuffer, uint32_t passID) {
    timeQuery.writeTimestampWithKey(cmdBuffer, passID);
}

void PipelineProfiler::resolveData(NativePipeline &pipeline) {

    const auto &timestampPeriod = pipeline.device->getCapabilities().timestampPeriod;
    std::vector<std::pair<RenderGraph::vertex_descriptor, uint64_t>> stack;
    timeQuery.foreachData([&](const auto &key, uint64_t v) {
        auto passID = ccstd::get<RenderGraph::vertex_descriptor>(key);
        if (stack.empty() || stack.back().first != passID) {
            stack.emplace_back(passID, v);
        } else {
            passTimes.emplace(passID, (v - stack.back().second) * timestampPeriod);
            stack.pop_back();
        }
    });

#if CC_USE_DEBUG_RENDERER
    auto *debugRenderer = DebugRenderer::getInstance();
    for (auto &[passID, time] : passTimes) {
        auto name = get(RenderGraph::NameTag{}, pipeline.renderGraph, passID);
    }
#endif
}


} // namespace cc::render
