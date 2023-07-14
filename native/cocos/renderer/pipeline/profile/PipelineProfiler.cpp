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
#include "base/StringUtil.h"
#include "application/ApplicationManager.h"
#include "platform/interfaces/modules/Device.h"
#include "platform/interfaces/modules/ISystemWindow.h"
#include "platform/interfaces/modules/ISystemWindowManager.h"

#include "gfx-base/GFXDevice.h"
#include "profiler/DebugRenderer.h"

#include "cocos/renderer/pipeline/custom/NativePipelineTypes.h"
#include "cocos/renderer/pipeline/custom/RenderGraphGraphs.h"



namespace cc::render {

PipelineProfiler::PipelineProfiler() {
    _device = gfx::Device::getInstance();
    _timestampPeriod = _device->getCapabilities().timestampPeriod;
#if CC_USE_DEBUG_RENDERER
    auto debugInfo = DebugRendererInfo();
    const auto *window = CC_GET_MAIN_SYSTEM_WINDOW();
    const auto &ext = window->getViewSize();
    _width = ext.width * Device::getDevicePixelRatio();
    _height = ext.height * Device::getDevicePixelRatio();
    auto fontSize = static_cast<uint32_t>(_width / 800.0F * static_cast<float>(debugInfo.fontSize));
    fontSize = fontSize < 20U ? 20U : (fontSize > 40U ? 40U : fontSize);

    _textRenderer = std::make_unique<TextRenderer>();
    _textRenderer->initialize(_device, debugInfo, fontSize);
    _textRenderer->updateWindowSize(static_cast<uint32_t>(_width), static_cast<uint32_t>(_height), 0, _device->getCombineSignY());

    initMaterial();
#endif
}

void PipelineProfiler::beginFrame(uint32_t passCount, gfx::CommandBuffer *cmdBuffer) {
    _timeQuery.resize(passCount * 2);
    _timeQuery.reset(cmdBuffer);
    _passTimes.clear();

    _max = 0;
    _min = std::numeric_limits<uint64_t>::max();
}

void PipelineProfiler::endFrame(gfx::CommandBuffer *cmdBuffer) {
    _timeQuery.copyResult(cmdBuffer);
}

void PipelineProfiler::writeGpuTimeStamp(gfx::CommandBuffer *cmdBuffer, uint32_t passID) {
    _timeQuery.writeTimestampWithKey(cmdBuffer, passID);
}

void PipelineProfiler::initMaterial() {
#if CC_USE_DEBUG_RENDERER
    IMaterialInfo matInfo = {};
    matInfo.effectName = "internal/builtin-debug-renderer";

    _material = ccnew Material();
    _material->setUuid("default-debug-renderer-material");
    _material->initialize(matInfo);
    _scenePass = (*_material->getPasses())[0];
#endif
}

void PipelineProfiler::render(gfx::RenderPass *renderPass, uint32_t subPassId, gfx::CommandBuffer *cmdBuff) {
#if CC_USE_DEBUG_RENDERER
    const auto *window = CC_GET_MAIN_SYSTEM_WINDOW();
    const auto &ext = window->getViewSize();
    _width = ext.width * Device::getDevicePixelRatio();
    _height = ext.height * Device::getDevicePixelRatio();
    _textRenderer->updateWindowSize(static_cast<uint32_t>(_width), static_cast<uint32_t>(_height), 0, _device->getCombineSignY());

    _textRenderer->updateTextData();
    _textRenderer->render(renderPass, subPassId, cmdBuff, _scenePass);
#endif
}

void PipelineProfiler::resolveData(NativePipeline &pipeline) {
    std::vector<std::pair<RenderGraph::vertex_descriptor, uint64_t>> stack;
    _timeQuery.foreachData([&](const auto &key, uint64_t v) {
        _min = std::min(_min, v);
        _max = std::max(_max, v);

        auto passID = ccstd::get<RenderGraph::vertex_descriptor>(key);
        if (stack.empty() || stack.back().first != passID) {
            _passTimes[passID].begin = v;
            stack.emplace_back(passID, v);
        } else {
            _passTimes[passID].end = v;
            stack.pop_back();
        }
    });

    auto timeFn = [this](uint64_t time) {
        return static_cast<float>(time - _min) / static_cast<float>(_max - _min);
    };

#if CC_USE_DEBUG_RENDERER
    const DebugTextInfo coreInfo = {{1.0F, 0.0F, 0.0F, 1.0F}, true, false, true, 1U, {0.0F, 0.0F, 0.0F, 1.0F}, 1.0F};
    const DebugTextInfo passInfo = {{1.0F, 1.0F, 1.0F, 1.0F}, true, false, false, 1U, {0.0F, 0.0F, 0.0F, 1.0F}, 1.0F};

    const uint32_t lineHeight = _textRenderer->getLineHeight();
    const auto offsetX = static_cast<uint32_t>(_width * 0.01F);
    auto offsetY = std::max(lineHeight, static_cast<uint32_t>(static_cast<float>(_height) * 0.01F));

    auto result = _textRenderer->addText("GPU Timeline",
                                         {static_cast<float>(offsetX), static_cast<float>(offsetY)},
                                         coreInfo);
    offsetY = static_cast<uint32_t>(result.y) + lineHeight;
    auto centerY = offsetY;

    Vec2 const backRectOffset = {static_cast<float>(offsetX), static_cast<float>(centerY - lineHeight * 1.5 / 2.0)};
    Vec2 const backRectExt = {_width - static_cast<float>(2 * offsetX), static_cast<float>(lineHeight * 1.5)};
    result = _textRenderer->addBackGroundRect(backRectOffset, backRectExt, gfx::Color{0.2F, 0.2F, 0.2F, 0.2F});
    offsetY = static_cast<uint32_t>(result.y) + lineHeight;

    uint32_t lines = 0;
    _textRenderer->addText("GPU CoreStats",
                           {static_cast<float>(offsetX), static_cast<float>(offsetY + lineHeight * lines++)},
                           coreInfo);

    for (auto &[passID, passTime] : _passTimes) {
        auto name = get(RenderGraph::NameTag{}, pipeline.renderGraph, passID);
        auto delta = static_cast<float>(static_cast<double>((passTime.end - passTime.begin) * _timestampPeriod) / static_cast<double>(1000000));

        float const x1 = backRectExt.x * timeFn(passTime.begin);
        float const x2 = backRectExt.x * timeFn(passTime.end);
        Vec2 const rectOffset = {static_cast<float>(offsetX) + x1, static_cast<float>(centerY - lineHeight / 2.0)};
        Vec2 const rectExt = {x2 - x1, static_cast<float>(lineHeight)};

        _textRenderer->addBackGroundRect(rectOffset, rectExt, gfx::Color{0.0F, 1.0F, 0.0F, 0.8F});

        result = _textRenderer->addText(StringUtil::format("%s", name.c_str()),
                                        {static_cast<float>(offsetX), static_cast<float>(offsetY + lineHeight * lines)},
                                        passInfo);
        _textRenderer->addText(StringUtil::format("[%f ms]", delta),
                               {static_cast<float>(offsetX) + std::max(result.x + _width / 50.F, 256.F), static_cast<float>(offsetY + lineHeight * lines++)},
                               passInfo);
    }
#endif
}


} // namespace cc::render
