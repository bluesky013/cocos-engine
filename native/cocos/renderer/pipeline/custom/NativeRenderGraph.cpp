/****************************************************************************
 Copyright (c) 2022-2023 Xiamen Yaji Software Co., Ltd.

 https://www.cocos.com/

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

#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/filtered_graph.hpp>
#include "LayoutGraphGraphs.h"
#include "NativePipelineGraphs.h"
#include "NativePipelineTypes.h"
#include "NativeUtils.h"
#include "RenderCommonNames.h"
#include "RenderCommonTypes.h"
#include "RenderGraphFwd.h"
#include "RenderGraphGraphs.h"
#include "RenderGraphTypes.h"
#include "RenderingModule.h"
#include "cocos/math/Utils.h"
#include "cocos/renderer/pipeline/Define.h"
#include "cocos/renderer/pipeline/PipelineUBO.h"
#include "cocos/scene/DirectionalLight.h"
#include "cocos/scene/Fog.h"
#include "cocos/scene/Skybox.h"
#include "cocos/scene/SpotLight.h"
#include "details/DebugUtils.h"
#include "details/GraphView.h"
#include "details/GslUtils.h"
#include "gfx-base/GFXDef-common.h"
#include "gfx-base/GFXDevice.h"
#include "pipeline/PipelineSceneData.h"

namespace cc {

namespace render {

ccstd::string NativeRenderNode::getName() const {
    return std::string(get(RenderGraph::NameTag{}, *renderGraph, nodeID));
}

void NativeRenderNode::setName(const ccstd::string &name) {
    get(RenderGraph::NameTag{}, *renderGraph, nodeID) = std::string_view{name};
}

void NativeSetter::setMat4(const ccstd::string &name, const Mat4 &mat) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setMat4Impl(data, *layoutGraph, name, mat);
}

void NativeSetter::setQuaternion(const ccstd::string &name, const Quaternion &quat) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setQuaternionImpl(data, *layoutGraph, name, quat);
}

void NativeSetter::setColor(const ccstd::string &name, const gfx::Color &color) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setColorImpl(data, *layoutGraph, name, color);
}

void NativeSetter::setVec4(const ccstd::string &name, const Vec4 &vec) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setVec4Impl(data, *layoutGraph, name, vec);
}

void NativeSetter::setVec2(const ccstd::string &name, const Vec2 &vec) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setVec2Impl(data, *layoutGraph, name, vec);
}

void NativeSetter::setFloat(const ccstd::string &name, float v) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setFloatImpl(data, *layoutGraph, name, v);
}

void NativeSetter::setArrayBuffer(const ccstd::string &name, const ArrayBuffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setArrayBufferImpl(data, *layoutGraph, name, *buffer);
}

void NativeSetter::setBuffer(const ccstd::string &name, gfx::Buffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setBufferImpl(data, *layoutGraph, name, buffer);
}

void NativeSetter::setTexture(const ccstd::string &name, gfx::Texture *texture) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setTextureImpl(data, *layoutGraph, name, texture);
}

void NativeSetter::setReadWriteBuffer(const ccstd::string &name, gfx::Buffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setReadWriteBufferImpl(data, *layoutGraph, name, buffer);
}

void NativeSetter::setReadWriteTexture(const ccstd::string &name, gfx::Texture *texture) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setReadWriteTextureImpl(data, *layoutGraph, name, texture);
}

void NativeSetter::setSampler(const ccstd::string &name, gfx::Sampler *sampler) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setSamplerImpl(data, *layoutGraph, name, sampler);
}

void NativeSetter::setVec4ArraySize(const ccstd::string &name, uint32_t sz) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setVec4ArraySizeImpl(data, *layoutGraph, name, sz);
}

void NativeSetter::setVec4ArrayElem(const ccstd::string &name, const cc::Vec4 &vec, uint32_t id) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setVec4ArrayElemImpl(data, *layoutGraph, name, vec, id);
}

void NativeSetter::setMat4ArraySize(const ccstd::string &name, uint32_t sz) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setMat4ArraySizeImpl(data, *layoutGraph, name, sz);
}

void NativeSetter::setMat4ArrayElem(const ccstd::string &name, const cc::Mat4 &mat, uint32_t id) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, nodeID);
    setMat4ArrayElemImpl(data, *layoutGraph, name, mat, id);
}

ccstd::string NativeRasterPassBuilder::getName() const {
    return std::string(get(RenderGraph::NameTag{}, *renderGraph, passID));
}

void NativeRasterPassBuilder::setName(const ccstd::string &name) {
    get(RenderGraph::NameTag{}, *renderGraph, passID) = std::string_view{name};
}

namespace {

template <class Tag>
void addRenderTargetImpl(
    RenderGraph &graph,
    RenderGraph::vertex_descriptor passID,
    std::string_view name,
    std::string_view slotName,
    AccessType accessType,
    gfx::LoadOp loadOp,
    gfx::StoreOp storeOp,
    const gfx::Color &color) {
    auto &pass = get(Tag{}, passID, graph);
    auto iter = pass.rasterViews.find(name);
    if (iter != pass.rasterViews.end()) {
        // no overwrite
        return;
    }
    auto slotID = static_cast<uint32_t>(pass.rasterViews.size());
    auto res = pass.rasterViews.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(name),
        std::forward_as_tuple(
            ccstd::pmr::string(slotName, graph.get_allocator()),
            accessType,
            AttachmentType::RENDER_TARGET,
            loadOp,
            storeOp,
            gfx::ClearFlagBit::COLOR,
            color));
    CC_ENSURES(res.second);
    res.first->second.slotID = slotID;
}

template <class Tag>
void addDepthStencilImpl(
    RenderGraph &graph,
    RenderGraph::vertex_descriptor passID,
    std::string_view name,
    std::string_view slotName,
    AccessType accessType,
    gfx::LoadOp loadOp,
    gfx::StoreOp storeOp,
    gfx::ClearFlagBit clearFlags,
    float depth,
    uint8_t stencil) {
    auto &pass = get(Tag{}, passID, graph);
    auto iter = pass.rasterViews.find(name);
    if (iter != pass.rasterViews.end()) {
        // no overwrite
        return;
    }
    auto slotID = static_cast<uint32_t>(pass.rasterViews.size());
    auto res = pass.rasterViews.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(name),
        std::forward_as_tuple(
            ccstd::pmr::string(slotName, graph.get_allocator()),
            accessType,
            AttachmentType::DEPTH_STENCIL,
            loadOp,
            storeOp,
            clearFlags,
            gfx::Color{depth, static_cast<float>(stencil), 0, 0}));
    CC_ENSURES(res.second);
    res.first->second.slotID = slotID;
}

} // namespace

void NativeRasterPassBuilder::addRenderTarget(
    const ccstd::string &name,
    const ccstd::string &slotName,
    gfx::LoadOp loadOp,
    gfx::StoreOp storeOp,
    const gfx::Color &color) {
    addRenderTargetImpl<RasterPassTag>(
        *renderGraph,
        passID,
        name,
        slotName,
        AccessType::WRITE,
        loadOp,
        storeOp,
        color);
}

void NativeRasterPassBuilder::addDepthStencil(
    const ccstd::string &name,
    const ccstd::string &slotName,
    gfx::LoadOp loadOp,
    gfx::StoreOp storeOp,
    float depth,
    uint8_t stencil,
    gfx::ClearFlagBit clearFlags) {
    addDepthStencilImpl<RasterPassTag>(
        *renderGraph,
        passID,
        name,
        slotName,
        AccessType::WRITE,
        loadOp,
        storeOp,
        clearFlags,
        depth,
        stencil);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void NativeRasterPassBuilder::addTexture(const ccstd::string &name, const ccstd::string &slotName) {
}

void NativeRasterPassBuilder::addStorageBuffer(const ccstd::string &name, AccessType accessType, const ccstd::string &slotName) {
}

void NativeRasterPassBuilder::addStorageImage(const ccstd::string &name, AccessType accessType, const ccstd::string &slotName) {
}

void NativeRasterPassBuilder::addRasterView(const ccstd::string &name, const RasterView &view) {
    auto &pass = get(RasterPassTag{}, passID, *renderGraph);
    auto slotID = static_cast<uint32_t>(pass.rasterViews.size());
    auto res = pass.rasterViews.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(name.c_str()),
        std::forward_as_tuple(view));
    CC_ENSURES(res.second);
    res.first->second.slotID = slotID;
}

void NativeRasterPassBuilder::addComputeView(const ccstd::string &name, const ComputeView &view) {
    CC_EXPECTS(!name.empty());
    CC_EXPECTS(!view.name.empty());
    auto &pass = get(RasterPassTag{}, passID, *renderGraph);
    auto iter = pass.computeViews.find(name.c_str());
    if (iter == pass.computeViews.end()) {
        bool added = false;
        std::tie(iter, added) = pass.computeViews.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(name.c_str()),
            std::forward_as_tuple());
        CC_ENSURES(added);
    }
    iter->second.emplace_back(view);
}

bool NativeRasterPassBuilder::getShowStatistics() const {
    const auto &pass = get(RasterPassTag{}, passID, *renderGraph);
    return pass.showStatistics;
}

void NativeRasterPassBuilder::setShowStatistics(bool enable) {
    auto &pass = get(RasterPassTag{}, passID, *renderGraph);
    pass.showStatistics = enable;
}

ccstd::string NativeRasterSubpassBuilder::getName() const {
    return std::string(get(RenderGraph::NameTag{}, *renderGraph, subpassID));
}

void NativeRasterSubpassBuilder::setName(const ccstd::string &name) {
    get(RenderGraph::NameTag{}, *renderGraph, subpassID) = std::string_view{name};
}

void NativeRasterSubpassBuilder::setMat4(const ccstd::string &name, const Mat4 &mat) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setMat4Impl(data, *layoutGraph, name, mat);
}

void NativeRasterSubpassBuilder::setQuaternion(const ccstd::string &name, const Quaternion &quat) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setQuaternionImpl(data, *layoutGraph, name, quat);
}

void NativeRasterSubpassBuilder::setColor(const ccstd::string &name, const gfx::Color &color) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setColorImpl(data, *layoutGraph, name, color);
}

void NativeRasterSubpassBuilder::setVec4(const ccstd::string &name, const Vec4 &vec) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setVec4Impl(data, *layoutGraph, name, vec);
}

void NativeRasterSubpassBuilder::setVec2(const ccstd::string &name, const Vec2 &vec) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setVec2Impl(data, *layoutGraph, name, vec);
}

void NativeRasterSubpassBuilder::setFloat(const ccstd::string &name, float v) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setFloatImpl(data, *layoutGraph, name, v);
}

void NativeRasterSubpassBuilder::setArrayBuffer(const ccstd::string &name, const ArrayBuffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setArrayBufferImpl(data, *layoutGraph, name, *buffer);
}

void NativeRasterSubpassBuilder::setBuffer(const ccstd::string &name, gfx::Buffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setBufferImpl(data, *layoutGraph, name, buffer);
}

void NativeRasterSubpassBuilder::setTexture(const ccstd::string &name, gfx::Texture *texture) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setTextureImpl(data, *layoutGraph, name, texture);
}

void NativeRasterSubpassBuilder::setReadWriteBuffer(const ccstd::string &name, gfx::Buffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setReadWriteBufferImpl(data, *layoutGraph, name, buffer);
}

void NativeRasterSubpassBuilder::setReadWriteTexture(const ccstd::string &name, gfx::Texture *texture) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setReadWriteTextureImpl(data, *layoutGraph, name, texture);
}

void NativeRasterSubpassBuilder::setSampler(const ccstd::string &name, gfx::Sampler *sampler) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setSamplerImpl(data, *layoutGraph, name, sampler);
}

namespace {

template <class Tag>
void addRasterViewImpl(
    std::string_view name,
    std::string_view slotName,
    AccessType accessType,
    AttachmentType attachmentType,
    gfx::LoadOp loadOp,
    gfx::StoreOp storeOp,
    gfx::ClearFlagBit clearFlags,
    gfx::Color clearColor,
    RenderGraph::vertex_descriptor subpassID,
    RenderGraph &renderGraph) {
    CC_EXPECTS(!name.empty());
    auto &subpass = get(Tag{}, subpassID, renderGraph);
    const auto slotID = static_cast<uint32_t>(subpass.rasterViews.size());
    const auto passID = parent(subpassID, renderGraph);
    CC_EXPECTS(passID != RenderGraph::null_vertex());
    CC_EXPECTS(holds<RasterPassTag>(passID, renderGraph));
    auto &pass = get(RasterPassTag{}, passID, renderGraph);
    CC_EXPECTS(subpass.subpassID < num_vertices(pass.subpassGraph));
    auto &subpassData = get(SubpassGraph::SubpassTag{}, pass.subpassGraph, subpass.subpassID);
    CC_EXPECTS(subpass.rasterViews.size() == subpassData.rasterViews.size());
    {
        auto res = subpassData.rasterViews.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(
                ccstd::pmr::string(slotName, subpassData.get_allocator()),
                accessType,
                attachmentType,
                loadOp,
                storeOp,
                clearFlags,
                clearColor));
        CC_ENSURES(res.second);
        res.first->second.slotID = slotID;
    }
    {
        auto res = subpass.rasterViews.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(
                ccstd::pmr::string(slotName, subpassData.get_allocator()),
                accessType,
                attachmentType,
                loadOp,
                storeOp,
                clearFlags,
                clearColor));
        CC_ENSURES(res.second);
        res.first->second.slotID = slotID;
    }
    CC_ENSURES(subpass.rasterViews.size() == subpassData.rasterViews.size());
}

template <class Tag>
void addComputeViewImpl(
    const ccstd::string &name, const ComputeView &view,
    RenderGraph::vertex_descriptor subpassID,
    RenderGraph &renderGraph) {
    CC_EXPECTS(!name.empty());
    CC_EXPECTS(!view.name.empty());
    auto &subpass = get(Tag{}, subpassID, renderGraph);
    const auto passID = parent(subpassID, renderGraph);
    CC_EXPECTS(passID != RenderGraph::null_vertex());
    CC_EXPECTS(holds<RasterPassTag>(passID, renderGraph));
    auto &pass = get(RasterPassTag{}, passID, renderGraph);
    CC_EXPECTS(subpass.subpassID < num_vertices(pass.subpassGraph));
    auto &subpassData = get(SubpassGraph::SubpassTag{}, pass.subpassGraph, subpass.subpassID);
    CC_EXPECTS(subpass.computeViews.size() == subpassData.computeViews.size());
    {
        auto iter = subpassData.computeViews.find(name.c_str());
        if (iter == subpassData.computeViews.end()) {
            bool added = false;
            std::tie(iter, added) = subpassData.computeViews.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(name.c_str()),
                std::forward_as_tuple());
            CC_ENSURES(added);
        }
        iter->second.emplace_back(view);
    }
    {
        auto iter = subpass.computeViews.find(name.c_str());
        if (iter == subpass.computeViews.end()) {
            bool added = false;
            std::tie(iter, added) = subpass.computeViews.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(name.c_str()),
                std::forward_as_tuple());
            CC_ENSURES(added);
        }
        iter->second.emplace_back(view);
    }
    CC_ENSURES(subpass.computeViews.size() == subpassData.computeViews.size());
    CC_ENSURES(subpass.computeViews.find(std::string_view{name}) != subpass.computeViews.end());
    CC_ENSURES(subpassData.computeViews.find(std::string_view{name}) != subpassData.computeViews.end());
    CC_ENSURES(subpass.computeViews.find(std::string_view{name})->second.size() ==
               subpassData.computeViews.find(std::string_view{name})->second.size());
}

} // namespace

void NativeRasterSubpassBuilder::addRenderTarget(
    const ccstd::string &name,
    AccessType accessType,
    const ccstd::string &slotName, gfx::LoadOp loadOp, gfx::StoreOp storeOp,
    const gfx::Color &color) {
    addRasterViewImpl<RasterSubpassTag>(
        name,
        slotName,
        accessType,
        AttachmentType::RENDER_TARGET,
        loadOp,
        storeOp,
        gfx::ClearFlagBit::COLOR,
        color,
        subpassID,
        *renderGraph);
}

void NativeRasterSubpassBuilder::addDepthStencil(
    const ccstd::string &name,
    AccessType accessType,
    const ccstd::string &slotName, gfx::LoadOp loadOp, gfx::StoreOp storeOp,
    float depth, uint8_t stencil, gfx::ClearFlagBit clearFlags) {
    addRasterViewImpl<RasterSubpassTag>(
        name,
        slotName,
        accessType,
        AttachmentType::DEPTH_STENCIL,
        loadOp,
        storeOp,
        clearFlags,
        gfx::Color{depth, static_cast<float>(stencil)},
        subpassID,
        *renderGraph);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void NativeRasterSubpassBuilder::addTexture(const ccstd::string &name, const ccstd::string &slotName) {
}

void NativeRasterSubpassBuilder::addStorageBuffer(const ccstd::string &name, AccessType accessType, const ccstd::string &slotName) {
}

void NativeRasterSubpassBuilder::addStorageImage(const ccstd::string &name, AccessType accessType, const ccstd::string &slotName) {
}

void NativeRasterSubpassBuilder::addComputeView(const ccstd::string &name, const ComputeView &view) {
    addComputeViewImpl<RasterSubpassTag>(name, view, subpassID, *renderGraph);
}

void NativeRasterSubpassBuilder::setViewport(const gfx::Viewport &viewport) {
    auto &subpass = get(RasterSubpassTag{}, subpassID, *renderGraph);
    subpass.viewport = viewport;
}

RasterQueueBuilder *NativeRasterSubpassBuilder::addQueue(QueueHint hint, const ccstd::string &layoutName) {
    CC_EXPECTS(layoutID != LayoutGraphData::null_vertex());

    auto phaseLayoutID = LayoutGraphData::null_vertex();
    if (!layoutName.empty()) {
        phaseLayoutID = locate(layoutID, layoutName, *layoutGraph);
        CC_ENSURES(phaseLayoutID != LayoutGraphData::null_vertex());
    }

    std::string_view name = "Queue";
    auto queueID = addVertex(
        QueueTag{},
        std::forward_as_tuple(name),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(hint, phaseLayoutID),
        *renderGraph, subpassID);

    return new NativeRasterQueueBuilder(pipelineRuntime, renderGraph, queueID, layoutGraph);
}

bool NativeRasterSubpassBuilder::getShowStatistics() const {
    const auto &subpass = get(RasterSubpassTag{}, subpassID, *renderGraph);
    return subpass.showStatistics;
}

void NativeRasterSubpassBuilder::setShowStatistics(bool enable) {
    auto &subpass = get(RasterSubpassTag{}, subpassID, *renderGraph);
    subpass.showStatistics = enable;
}

ccstd::string NativeComputeSubpassBuilder::getName() const {
    return std::string(get(RenderGraph::NameTag{}, *renderGraph, subpassID));
}

void NativeComputeSubpassBuilder::setName(const ccstd::string &name) {
    get(RenderGraph::NameTag{}, *renderGraph, subpassID) = std::string_view{name};
}

void NativeComputeSubpassBuilder::setMat4(const ccstd::string &name, const Mat4 &mat) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setMat4Impl(data, *layoutGraph, name, mat);
}

void NativeComputeSubpassBuilder::setQuaternion(const ccstd::string &name, const Quaternion &quat) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setQuaternionImpl(data, *layoutGraph, name, quat);
}

void NativeComputeSubpassBuilder::setColor(const ccstd::string &name, const gfx::Color &color) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setColorImpl(data, *layoutGraph, name, color);
}

void NativeComputeSubpassBuilder::setVec4(const ccstd::string &name, const Vec4 &vec) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setVec4Impl(data, *layoutGraph, name, vec);
}

void NativeComputeSubpassBuilder::setVec2(const ccstd::string &name, const Vec2 &vec) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setVec2Impl(data, *layoutGraph, name, vec);
}

void NativeComputeSubpassBuilder::setFloat(const ccstd::string &name, float v) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setFloatImpl(data, *layoutGraph, name, v);
}

void NativeComputeSubpassBuilder::setArrayBuffer(const ccstd::string &name, const ArrayBuffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setArrayBufferImpl(data, *layoutGraph, name, *buffer);
}

void NativeComputeSubpassBuilder::setBuffer(const ccstd::string &name, gfx::Buffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setBufferImpl(data, *layoutGraph, name, buffer);
}

void NativeComputeSubpassBuilder::setTexture(const ccstd::string &name, gfx::Texture *texture) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setTextureImpl(data, *layoutGraph, name, texture);
}

void NativeComputeSubpassBuilder::setReadWriteBuffer(const ccstd::string &name, gfx::Buffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setReadWriteBufferImpl(data, *layoutGraph, name, buffer);
}

void NativeComputeSubpassBuilder::setReadWriteTexture(const ccstd::string &name, gfx::Texture *texture) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setReadWriteTextureImpl(data, *layoutGraph, name, texture);
}

void NativeComputeSubpassBuilder::setSampler(const ccstd::string &name, gfx::Sampler *sampler) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, subpassID);
    setSamplerImpl(data, *layoutGraph, name, sampler);
}

void NativeComputeSubpassBuilder::addRenderTarget(const ccstd::string &name, const ccstd::string &slotName) {
    addRasterViewImpl<ComputeSubpassTag>(
        name,
        slotName,
        AccessType::READ,
        AttachmentType::RENDER_TARGET,
        gfx::LoadOp::LOAD,
        gfx::StoreOp::STORE,
        gfx::ClearFlagBit::NONE,
        gfx::Color{},
        subpassID,
        *renderGraph);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void NativeComputeSubpassBuilder::addTexture(const ccstd::string &name, const ccstd::string &slotName) {
}

void NativeComputeSubpassBuilder::addStorageBuffer(const ccstd::string &name, AccessType accessType, const ccstd::string &slotName) {
}

void NativeComputeSubpassBuilder::addStorageImage(const ccstd::string &name, AccessType accessType, const ccstd::string &slotName) {
}

void NativeComputeSubpassBuilder::addComputeView(const ccstd::string &name, const ComputeView &view) {
    addComputeViewImpl<ComputeSubpassTag>(name, view, subpassID, *renderGraph);
}

ComputeQueueBuilder *NativeComputeSubpassBuilder::addQueue(const ccstd::string &layoutName) {
    CC_EXPECTS(layoutID != LayoutGraphData::null_vertex());

    auto phaseLayoutID = LayoutGraphData::null_vertex();
    if (!layoutName.empty()) {
        phaseLayoutID = locate(layoutID, layoutName, *layoutGraph);
        CC_ENSURES(phaseLayoutID != LayoutGraphData::null_vertex());
    }

    std::string_view name = "Queue";
    auto queueID = addVertex(
        QueueTag{},
        std::forward_as_tuple(name),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(phaseLayoutID),
        *renderGraph, subpassID);

    return new NativeComputeQueueBuilder(renderGraph, queueID, layoutGraph);
}

namespace {

uint8_t getCombineSignY(gfx::Device *device) {
    // 0: vk, 1: metal, 2: none, 3: gl-like
    static int8_t combineSignY{-1};
    if (combineSignY < 0) {
        const float screenSpaceSignY = device->getCapabilities().screenSpaceSignY * 0.5F + 0.5F;
        const float clipSpaceSignY = device->getCapabilities().clipSpaceSignY * 0.5F + 0.5F;
        combineSignY = static_cast<int8_t>(static_cast<int>(screenSpaceSignY) << 1 | static_cast<int>(clipSpaceSignY));
    }
    return static_cast<uint8_t>(combineSignY);
}

void setCameraUBOValues(
    const scene::Camera &camera,
    const LayoutGraphData &layoutGraph,
    const pipeline::PipelineSceneData &cfg,
    const scene::DirectionalLight *mainLight,
    RenderData &data) {
    CC_EXPECTS(camera.getNode());
    CC_EXPECTS(cfg.getSkybox());
    const auto &skybox = *cfg.getSkybox();
    const auto &shadingScale = cfg.getShadingScale();
    // Camera
    setMat4Impl(data, layoutGraph, "cc_matView", camera.getMatView());
    setMat4Impl(data, layoutGraph, "cc_matViewInv", camera.getNode()->getWorldMatrix());
    setMat4Impl(data, layoutGraph, "cc_matProj", camera.getMatProj());
    setMat4Impl(data, layoutGraph, "cc_matProjInv", camera.getMatProjInv());
    setMat4Impl(data, layoutGraph, "cc_matViewProj", camera.getMatViewProj());
    setMat4Impl(data, layoutGraph, "cc_matViewProjInv", camera.getMatViewProjInv());
    setVec4Impl(data, layoutGraph, "cc_cameraPos",
                Vec4(
                    camera.getPosition().x,
                    camera.getPosition().y,
                    camera.getPosition().z,
                    getCombineSignY(cc::gfx::Device::getInstance())));
    setVec4Impl(data, layoutGraph, "cc_surfaceTransform",
                Vec4(
                    static_cast<float>(camera.getSurfaceTransform()),
                    0.0F,
                    cosf(static_cast<float>(mathutils::toRadian(skybox.getRotationAngle()))),
                    sinf(static_cast<float>(mathutils::toRadian(skybox.getRotationAngle())))));
    setVec4Impl(data, layoutGraph, "cc_screenScale",
                Vec4(
                    cfg.getShadingScale(),
                    cfg.getShadingScale(),
                    1.0F / cfg.getShadingScale(),
                    1.0F / cfg.getShadingScale()));
    setVec4Impl(data, layoutGraph, "cc_exposure",
                Vec4(
                    camera.getExposure(),
                    1.0F / camera.getExposure(),
                    cfg.isHDR() ? 1.0F : 0.0F,
                    1.0F / scene::Camera::getStandardExposureValue()));

    if (mainLight) {
        const auto &shadowInfo = *cfg.getShadows();
        const bool shadowEnable = (mainLight->isShadowEnabled() &&
                                   shadowInfo.getType() == scene::ShadowType::SHADOW_MAP);
        setVec4Impl(data, layoutGraph, "cc_mainLitDir",
                    Vec4(
                        mainLight->getDirection().x,
                        mainLight->getDirection().y,
                        mainLight->getDirection().z,
                        shadowEnable));
        auto r = mainLight->getColor().x;
        auto g = mainLight->getColor().y;
        auto b = mainLight->getColor().z;
        if (mainLight->isUseColorTemperature()) {
            r *= mainLight->getColorTemperatureRGB().x;
            g *= mainLight->getColorTemperatureRGB().y;
            b *= mainLight->getColorTemperatureRGB().z;
        }
        auto w = mainLight->getIlluminance();
        if (cfg.isHDR()) {
            w *= camera.getExposure();
        }
        setVec4Impl(data, layoutGraph, "cc_mainLitColor", Vec4(r, g, b, w));
    } else {
        setVec4Impl(data, layoutGraph, "cc_mainLitDir", Vec4(0, 0, 1, 0));
        setVec4Impl(data, layoutGraph, "cc_mainLitColor", Vec4(0, 0, 0, 0));
    }

    CC_EXPECTS(cfg.getAmbient());
    auto &ambient = *cfg.getAmbient();
    auto &skyColor = ambient.getSkyColor();
    if (cfg.isHDR()) {
        skyColor.w = ambient.getSkyIllum() * camera.getExposure();
    } else {
        skyColor.w = ambient.getSkyIllum();
    }
    setVec4Impl(data, layoutGraph, "cc_ambientSky",
                Vec4(skyColor.x, skyColor.y, skyColor.z, skyColor.w));
    setVec4Impl(data, layoutGraph, "cc_ambientGround",
                Vec4(
                    ambient.getGroundAlbedo().x,
                    ambient.getGroundAlbedo().y,
                    ambient.getGroundAlbedo().z,
                    skybox.getEnvmap() ? static_cast<float>(skybox.getEnvmap()->mipmapLevel()) : 1.0F));

    CC_EXPECTS(cfg.getFog());
    const auto &fog = *cfg.getFog();

    const auto &colorTempRGB = fog.getColorArray();
    setVec4Impl(data, layoutGraph, "cc_fogColor",
                Vec4(colorTempRGB.x, colorTempRGB.y, colorTempRGB.z, colorTempRGB.z));
    setVec4Impl(data, layoutGraph, "cc_fogBase",
                Vec4(fog.getFogStart(), fog.getFogEnd(), fog.getFogDensity(), 0.0F));
    setVec4Impl(data, layoutGraph, "cc_fogAdd",
                Vec4(fog.getFogTop(), fog.getFogRange(), fog.getFogAtten(), 0.0F));
    setVec4Impl(data, layoutGraph, "cc_nearFar",
                Vec4(camera.getNearClip(), camera.getFarClip(), 0.0F, 0.0F));
    setVec4Impl(data, layoutGraph, "cc_viewPort",
                Vec4(
                    camera.getViewport().x,
                    camera.getViewport().y,
                    shadingScale * static_cast<float>(camera.getWindow()->getWidth()) * camera.getViewport().z,
                    shadingScale * static_cast<float>(camera.getWindow()->getHeight()) * camera.getViewport().w));
}

void setShadowUBOLightView(
    gfx::Device *device,
    const LayoutGraphData &layoutGraph,
    const pipeline::PipelineSceneData &sceneData,
    const scene::Light &light,
    uint32_t level,
    RenderData &data) {
    const auto &shadowInfo = *sceneData.getShadows();
    const auto &csmLayers = *sceneData.getCSMLayers();
    const auto &packing = pipeline::supportsR32FloatTexture(device) ? 0.0F : 1.0F;
    const auto &cap = device->getCapabilities();
    Vec4 vec4ShadowInfo{};

    // ShadowMap
    switch (light.getType()) {
        case scene::LightType::DIRECTIONAL: {
            const auto &mainLight = dynamic_cast<const scene::DirectionalLight &>(light);
            if (shadowInfo.isEnabled() && mainLight.isShadowEnabled()) {
                if (shadowInfo.getType() == scene::ShadowType::SHADOW_MAP) {
                    float near = 0.1;
                    float far = 0;
                    Mat4 matShadowView;
                    Mat4 matShadowProj;
                    Mat4 matShadowViewProj;
                    scene::CSMLevel levelCount{};
                    if (mainLight.isShadowFixedArea() || mainLight.getCSMLevel() == scene::CSMLevel::LEVEL_1) {
                        matShadowView = csmLayers.getSpecialLayer()->getMatShadowView();
                        matShadowProj = csmLayers.getSpecialLayer()->getMatShadowProj();
                        matShadowViewProj = csmLayers.getSpecialLayer()->getMatShadowViewProj();
                        if (mainLight.isShadowFixedArea()) {
                            near = mainLight.getShadowNear();
                            far = mainLight.getShadowFar();
                            levelCount = static_cast<scene::CSMLevel>(0);
                        } else {
                            near = 0.1;
                            far = csmLayers.getSpecialLayer()->getShadowCameraFar();
                            levelCount = scene::CSMLevel::LEVEL_1;
                        }
                        vec4ShadowInfo.set(0.0F, packing, mainLight.getShadowNormalBias(), 0);
                        setVec4Impl(data, layoutGraph, "cc_shadowLPNNInfo", vec4ShadowInfo);
                    } else {
                        const auto &layer = *csmLayers.getLayers()[level];
                        matShadowView = layer.getMatShadowView();
                        matShadowProj = layer.getMatShadowProj();
                        matShadowViewProj = layer.getMatShadowViewProj();

                        near = layer.getSplitCameraNear();
                        far = layer.getSplitCameraFar();
                        levelCount = mainLight.getCSMLevel();
                    }
                    setMat4Impl(data, layoutGraph, "cc_matLightView", matShadowView);
                    setVec4Impl(data, layoutGraph, "cc_shadowProjDepthInfo",
                                Vec4(
                                    matShadowProj.m[10],
                                    matShadowProj.m[14],
                                    matShadowProj.m[11],
                                    matShadowProj.m[15]));
                    setVec4Impl(data, layoutGraph, "cc_shadowProjInfo",
                                Vec4(
                                    matShadowProj.m[00],
                                    matShadowProj.m[05],
                                    1.0F / matShadowProj.m[00],
                                    1.0F / matShadowProj.m[05]));
                    setMat4Impl(data, layoutGraph, "cc_matLightViewProj", matShadowViewProj);
                    vec4ShadowInfo.set(near, far, 0, 1.0F - mainLight.getShadowSaturation());
                    setVec4Impl(data, layoutGraph, "cc_shadowNFLSInfo", vec4ShadowInfo);
                    vec4ShadowInfo.set(
                        0.0F,
                        packing,
                        mainLight.getShadowNormalBias(),
                        static_cast<float>(levelCount));
                    setVec4Impl(data, layoutGraph, "cc_shadowLPNNInfo", vec4ShadowInfo);
                    vec4ShadowInfo.set(
                        shadowInfo.getSize().x,
                        shadowInfo.getSize().y,
                        static_cast<float>(mainLight.getShadowPcf()),
                        mainLight.getShadowBias());
                    setVec4Impl(data, layoutGraph, "cc_shadowWHPBInfo", vec4ShadowInfo);
                }
            }
            break;
        }
        case scene::LightType::SPOT: {
            const auto &spotLight = dynamic_cast<const scene::SpotLight &>(light);
            if (shadowInfo.isEnabled() && spotLight.isShadowEnabled()) {
                const auto &matShadowCamera = spotLight.getNode()->getWorldMatrix();
                const auto matShadowView = matShadowCamera.getInversed();
                setMat4Impl(data, layoutGraph, "cc_matLightView", matShadowView);

                Mat4 matShadowViewProj{};
                Mat4::createPerspective(spotLight.getAngle(), 1.0F, 0.001F,
                                        spotLight.getRange(), true,
                                        cap.clipSpaceMinZ, cap.clipSpaceSignY, 0, &matShadowViewProj);
                matShadowViewProj.multiply(matShadowView);
                setMat4Impl(data, layoutGraph, "cc_matLightViewProj", matShadowViewProj);

                const Vec4 shadowNFLSInfos(0.01F, spotLight.getRange(), 0.0F, 0.0F);
                setVec4Impl(data, layoutGraph, "cc_shadowNFLSInfo", shadowNFLSInfos);

                const Vec4 shadowWHPBInfos(
                    shadowInfo.getSize().x,
                    shadowInfo.getSize().y,
                    spotLight.getShadowPcf(),
                    spotLight.getShadowBias());
                setVec4Impl(data, layoutGraph, "cc_shadowWHPBInfo", shadowWHPBInfos);

                const Vec4 shadowLPNNInfos(1.0F, packing, spotLight.getShadowNormalBias(), 0.0F);
                setVec4Impl(data, layoutGraph, "cc_shadowLPNNInfo", shadowLPNNInfos);
            }
            break;
        }
        default:
            break;
    }

    const auto &color = shadowInfo.getShadowColor4f();
    setColorImpl(data, layoutGraph, "cc_shadowColor", gfx::Color{color[0], color[1], color[2], color[3]});
}

float getPCFRadius(
    const scene::Shadows &shadowInfo,
    const scene::DirectionalLight &mainLight) {
    const auto &shadowMapSize = shadowInfo.getSize().x;
    switch (mainLight.getShadowPcf()) {
        case scene::PCFType::HARD:
            return 0.0F;
        case scene::PCFType::SOFT:
            return 1.0F / (shadowMapSize * 0.5F);
        case scene::PCFType::SOFT_2X:
            return 2.0F / (shadowMapSize * 0.5F);
        // case PCFType.SOFT_4X:
        //     return 3.0F  / (shadowMapSize * 0.5F);
        default:
            break;
    }
    return 0.0F;
}

void setShadowUBOView(
    gfx::Device &device,
    const LayoutGraphData &layoutGraph,
    const pipeline::PipelineSceneData &sceneData,
    const scene::DirectionalLight &mainLight,
    RenderData &data) {
    const auto &shadowInfo = *sceneData.getShadows();
    const auto &csmLayers = *sceneData.getCSMLayers();
    const auto &csmSupported = sceneData.getCSMSupported();
    const auto &packing = pipeline::supportsR32FloatTexture(&device) ? 0.0F : 1.0F;
    Vec4 vec4ShadowInfo{};
    if (shadowInfo.isEnabled()) {
        if (shadowInfo.getType() == scene::ShadowType::SHADOW_MAP) {
            if (mainLight.isShadowEnabled()) {
                if (mainLight.isShadowFixedArea() ||
                    mainLight.getCSMLevel() == scene::CSMLevel::LEVEL_1 || !csmSupported) {
                    // Shadow
                    const auto &matShadowView = csmLayers.getSpecialLayer()->getMatShadowView();
                    const auto &matShadowProj = csmLayers.getSpecialLayer()->getMatShadowProj();
                    const auto &matShadowViewProj = csmLayers.getSpecialLayer()->getMatShadowViewProj();
                    const auto &near = mainLight.getShadowNear();
                    const auto &far = mainLight.getShadowFar();

                    setMat4Impl(data, layoutGraph, "cc_matLightView", matShadowView);
                    setVec4Impl(data, layoutGraph, "cc_shadowProjDepthInfo",
                                Vec4(matShadowProj.m[10], matShadowProj.m[14],
                                     matShadowProj.m[11], matShadowProj.m[15]));

                    setVec4Impl(data, layoutGraph, "cc_shadowProjInfo",
                                Vec4(matShadowProj.m[00], matShadowProj.m[05],
                                     1.0F / matShadowProj.m[00], 1.0F / matShadowProj.m[05]));
                    setMat4Impl(data, layoutGraph, "cc_matLightViewProj", matShadowViewProj);
                    vec4ShadowInfo.set(near, far, 0, 1.0F - mainLight.getShadowSaturation());
                    setVec4Impl(data, layoutGraph, "cc_shadowNFLSInfo", vec4ShadowInfo);
                    vec4ShadowInfo.set(0.0F, packing, mainLight.getShadowNormalBias(), 0);
                    setVec4Impl(data, layoutGraph, "cc_shadowLPNNInfo", vec4ShadowInfo);
                } else {
                    { // CSM
                        const auto layerThreshold = getPCFRadius(shadowInfo, mainLight);
                        const auto numCascades = static_cast<uint32_t>(mainLight.getCSMLevel());
                        setVec4ArraySizeImpl(data, layoutGraph, "cc_csmViewDir0", numCascades);
                        setVec4ArraySizeImpl(data, layoutGraph, "cc_csmViewDir1", numCascades);
                        setVec4ArraySizeImpl(data, layoutGraph, "cc_csmViewDir2", numCascades);
                        setVec4ArraySizeImpl(data, layoutGraph, "cc_csmAtlas", numCascades);
                        setMat4ArraySizeImpl(data, layoutGraph, "cc_matCSMViewProj", numCascades);
                        setVec4ArraySizeImpl(data, layoutGraph, "cc_csmProjDepthInfo", numCascades);
                        setVec4ArraySizeImpl(data, layoutGraph, "cc_csmProjInfo", numCascades);

                        Vec4 csmSplitsInfo{};
                        for (uint32_t i = 0; i < numCascades; ++i) {
                            const auto &layer = *csmLayers.getLayers()[i];

                            const auto &matShadowView = layer.getMatShadowView();
                            vec4ShadowInfo.set(matShadowView.m[0], matShadowView.m[4], matShadowView.m[8], layerThreshold);
                            setVec4ArrayElemImpl(data, layoutGraph, "cc_csmViewDir0", vec4ShadowInfo, i);
                            vec4ShadowInfo.set(matShadowView.m[1], matShadowView.m[5], matShadowView.m[9], 0.0F);
                            setVec4ArrayElemImpl(data, layoutGraph, "cc_csmViewDir1", vec4ShadowInfo, i);
                            vec4ShadowInfo.set(matShadowView.m[2], matShadowView.m[6], matShadowView.m[0], 0.0F);
                            setVec4ArrayElemImpl(data, layoutGraph, "cc_csmViewDir2", vec4ShadowInfo, i);

                            const auto &csmAtlas = layer.getCSMAtlas();
                            setVec4ArrayElemImpl(data, layoutGraph, "cc_csmAtlas", csmAtlas, i);

                            const auto &matShadowViewProj = layer.getMatShadowViewProj();
                            setMat4ArrayElemImpl(data, layoutGraph, "cc_matCSMViewProj", matShadowViewProj, i);

                            const auto &matShadowProj = layer.getMatShadowProj();
                            setVec4ArrayElemImpl(data, layoutGraph,
                                                 "cc_csmProjDepthInfo",
                                                 Vec4(matShadowProj.m[10], matShadowProj.m[14],
                                                      matShadowProj.m[11], matShadowProj.m[15]),
                                                 i);

                            setVec4ArrayElemImpl(data, layoutGraph,
                                                 "cc_csmProjInfo",
                                                 Vec4(matShadowProj.m[00], matShadowProj.m[05],
                                                      1.0F / matShadowProj.m[00], 1.0F / matShadowProj.m[05]),
                                                 i);

                            (&csmSplitsInfo.x)[i] = layer.getSplitCameraFar() / mainLight.getShadowDistance();
                        }
                        setVec4Impl(data, layoutGraph, "cc_csmSplitsInfo", csmSplitsInfo);
                    }
                    { // Shadow
                        vec4ShadowInfo.set(0, 0, 0, 1.0F - mainLight.getShadowSaturation());
                        setVec4Impl(data, layoutGraph, "cc_shadowNFLSInfo", vec4ShadowInfo);
                        vec4ShadowInfo.set(
                            0.0F, packing,
                            mainLight.getShadowNormalBias(),
                            static_cast<float>(mainLight.getCSMLevel()));
                        setVec4Impl(data, layoutGraph, "cc_shadowLPNNInfo", vec4ShadowInfo);
                    }
                }
                { // Shadow
                    vec4ShadowInfo.set(
                        shadowInfo.getSize().x, shadowInfo.getSize().y,
                        static_cast<float>(mainLight.getShadowPcf()), mainLight.getShadowBias());
                    setVec4Impl(data, layoutGraph, "cc_shadowWHPBInfo", vec4ShadowInfo);
                }
            }
        } else {
            Vec3 tempVec3 = shadowInfo.getNormal().getNormalized();
            setVec4Impl(data, layoutGraph,
                        "cc_planarNDInfo",
                        Vec4(tempVec3.x, tempVec3.y, tempVec3.z, -shadowInfo.getDistance()));
        }
        {
            const auto &color = shadowInfo.getShadowColor4f();
            setColorImpl(data, layoutGraph, "cc_shadowColor",
                         gfx::Color{color[0], color[1], color[2], color[3]});
        }
    }
}

void setTextureUBOView(
    gfx::Device &device,
    const LayoutGraphData &layoutGraph,
    const pipeline::PipelineSceneData &sceneData,
    RenderData &data) {
    const auto &skybox = *sceneData.getSkybox();
    if (skybox.getReflectionMap()) {
        auto &texture = *skybox.getReflectionMap()->getGFXTexture();
        auto *sampler = device.getSampler(skybox.getReflectionMap()->getSamplerInfo());
        setTextureImpl(data, layoutGraph, "cc_environment", &texture);
        setSamplerImpl(data, layoutGraph, "cc_environment", sampler);
    } else {
        const auto *envmap =
            skybox.getEnvmap()
                ? skybox.getEnvmap()
                : BuiltinResMgr::getInstance()->get<TextureCube>("default-cube-texture");
        if (envmap) {
            auto *texture = envmap->getGFXTexture();
            auto *sampler = device.getSampler(envmap->getSamplerInfo());
            setTextureImpl(data, layoutGraph, "cc_environment", texture);
            setSamplerImpl(data, layoutGraph, "cc_environment", sampler);
        }
    }
    const auto *diffuseMap =
        skybox.getDiffuseMap()
            ? skybox.getDiffuseMap()
            : BuiltinResMgr::getInstance()->get<TextureCube>("default-cube-texture");
    if (diffuseMap) {
        auto *texture = diffuseMap->getGFXTexture();
        auto *sampler = device.getSampler(diffuseMap->getSamplerInfo());
        setTextureImpl(data, layoutGraph, "cc_diffuseMap", texture);
        setSamplerImpl(data, layoutGraph, "cc_diffuseMap", sampler);
    }
    gfx::SamplerInfo samplerPointInfo{
        gfx::Filter::POINT,
        gfx::Filter::POINT,
        gfx::Filter::NONE,
        gfx::Address::CLAMP,
        gfx::Address::CLAMP,
        gfx::Address::CLAMP};
    auto *pointSampler = device.getSampler(samplerPointInfo);
    setSamplerImpl(data, layoutGraph, "cc_shadowMap", pointSampler);
    setTextureImpl(data, layoutGraph, "cc_shadowMap", BuiltinResMgr::getInstance()->get<Texture2D>("default-texture")->getGFXTexture());
    setSamplerImpl(data, layoutGraph, "cc_spotShadowMap", pointSampler);
    setTextureImpl(data, layoutGraph, "cc_spotShadowMap", BuiltinResMgr::getInstance()->get<Texture2D>("default-texture")->getGFXTexture());
}

} // namespace

void NativeRasterQueueBuilder::addSceneOfCamera(
    scene::Camera *camera, LightInfo light, SceneFlags sceneFlags) {
    std::string_view name = "Camera";
    auto *pLight = light.light.get();
    SceneData scene(renderGraph->get_allocator());
    scene.name = name;
    scene.flags = sceneFlags;
    scene.camera = camera;
    scene.light = light;
    auto sceneID = addVertex(
        SceneTag{},
        std::forward_as_tuple(name),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(std::move(scene)),
        *renderGraph, nodeID);
    CC_ENSURES(sceneID != RenderGraph::null_vertex());

    auto &data = get(RenderGraph::DataTag{}, *renderGraph, sceneID);

    setCameraUBOValues(
        *camera,
        *layoutGraph,
        *pipelineRuntime->getPipelineSceneData(),
        camera->getScene()->getMainLight(), data);

    if (any(sceneFlags & SceneFlags::SHADOW_CASTER)) {
        if (pLight) {
            setShadowUBOLightView(
                pipelineRuntime->getDevice(),
                *layoutGraph,
                *pipelineRuntime->getPipelineSceneData(),
                *pLight, light.level, data);
        }
    } else {
        const auto *pDirLight = camera->getScene()->getMainLight();
        if (pDirLight) {
            setShadowUBOView(*pipelineRuntime->getDevice(),
                             *layoutGraph,
                             *pipelineRuntime->getPipelineSceneData(),
                             *pDirLight, data);
        }
    }
    setTextureUBOView(
        *pipelineRuntime->getDevice(),
        *layoutGraph,
        *pipelineRuntime->getPipelineSceneData(),
        data);
}

void NativeRasterQueueBuilder::addScene(const ccstd::string &name, SceneFlags sceneFlags) {
    SceneData scene(renderGraph->get_allocator());
    scene.name = name;
    scene.flags = sceneFlags;

    auto sceneID = addVertex(
        SceneTag{},
        std::forward_as_tuple(name.c_str()),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(std::move(scene)),
        *renderGraph, nodeID);
    CC_ENSURES(sceneID != RenderGraph::null_vertex());
}

void NativeRasterQueueBuilder::addFullscreenQuad(
    Material *material, uint32_t passID, SceneFlags sceneFlags) {
    std::string_view name = "FullscreenQuad";
    auto drawID = addVertex(
        BlitTag{},
        std::forward_as_tuple(name),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(material, passID, sceneFlags, nullptr),
        *renderGraph, nodeID);
    CC_ENSURES(drawID != RenderGraph::null_vertex());
}

void NativeRasterQueueBuilder::addCameraQuad(
    scene::Camera *camera, cc::Material *material, uint32_t passID,
    SceneFlags sceneFlags) {
    std::string_view name = "CameraQuad";
    auto drawID = addVertex(
        BlitTag{},
        std::forward_as_tuple(name),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(material, passID, sceneFlags, camera),
        *renderGraph, nodeID);
    CC_ENSURES(drawID != RenderGraph::null_vertex());

    auto &data = get(RenderGraph::DataTag{}, *renderGraph, drawID);

    setCameraUBOValues(
        *camera,
        *layoutGraph,
        *pipelineRuntime->getPipelineSceneData(),
        camera->getScene()->getMainLight(), data);

    if (any(sceneFlags & SceneFlags::SHADOW_CASTER)) {
    } else {
        const auto *pDirLight = camera->getScene()->getMainLight();
        if (pDirLight) {
            setShadowUBOView(*pipelineRuntime->getDevice(),
                             *layoutGraph,
                             *pipelineRuntime->getPipelineSceneData(),
                             *pDirLight, data);
        }
    }
    setTextureUBOView(
        *pipelineRuntime->getDevice(),
        *layoutGraph,
        *pipelineRuntime->getPipelineSceneData(),
        data);
}

void NativeRasterQueueBuilder::clearRenderTarget(const ccstd::string &name, const gfx::Color &color) {
    ccstd::pmr::vector<ClearView> clears(renderGraph->get_allocator());
    clears.emplace_back(name.c_str(), gfx::ClearFlagBit::COLOR, color);

    auto clearID = addVertex(
        ClearTag{},
        std::forward_as_tuple("ClearRenderTarget"),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(std::move(clears)),
        *renderGraph, nodeID);
    CC_ENSURES(clearID != RenderGraph::null_vertex());
}

void NativeRasterQueueBuilder::setViewport(const gfx::Viewport &viewport) {
    auto viewportID = addVertex(
        ViewportTag{},
        std::forward_as_tuple("Viewport"),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(viewport),
        *renderGraph, nodeID);
    CC_ENSURES(viewportID != RenderGraph::null_vertex());
}

RasterQueueBuilder *NativeRasterPassBuilder::addQueue(
    QueueHint hint, const ccstd::string &layoutName) {
    CC_EXPECTS(layoutID != LayoutGraphData::null_vertex());

    auto phaseLayoutID = LayoutGraphData::null_vertex();
    if (!layoutName.empty()) {
        phaseLayoutID = locate(layoutID, layoutName, *layoutGraph);
        CC_ENSURES(phaseLayoutID != LayoutGraphData::null_vertex());
    }

    std::string_view name = "Queue";
    auto queueID = addVertex(
        QueueTag{},
        std::forward_as_tuple(name),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(hint, phaseLayoutID),
        *renderGraph, passID);

    return new NativeRasterQueueBuilder(pipelineRuntime, renderGraph, queueID, layoutGraph);
}

RasterSubpassBuilder *NativeRasterPassBuilder::addRasterSubpass(const ccstd::string &layoutName) {
    std::string_view name("RasterSubpass");
    auto &pass = get(RasterPassTag{}, passID, *renderGraph);
    auto &subpassGraph = pass.subpassGraph;
    const auto subpassIndex = num_vertices(pass.subpassGraph);
    {
        auto id = addVertex(
            std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(),
            subpassGraph);
        CC_ENSURES(id == subpassIndex);
    }

    RasterSubpass subpass(subpassIndex, renderGraph->get_allocator());
    subpass.viewport.width = pass.width;
    subpass.viewport.height = pass.height;

    auto subpassID = addVertex(
        RasterSubpassTag{},
        std::forward_as_tuple(name),
        std::forward_as_tuple(layoutName.c_str()),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(std::move(subpass)),
        *renderGraph, passID);

    auto subpassLayoutID = locate(LayoutGraphData::null_vertex(), layoutName, *layoutGraph);
    CC_EXPECTS(subpassLayoutID != LayoutGraphData::null_vertex());

    auto *builder = ccnew NativeRasterSubpassBuilder(
        pipelineRuntime, renderGraph, subpassID, layoutGraph, subpassLayoutID);
    updateRasterPassConstants(pass.width, pass.height, *builder);

    return builder;
}

ComputeSubpassBuilder *NativeRasterPassBuilder::addComputeSubpass(const ccstd::string &layoutName) {
    std::string_view name("ComputeSubpass");
    auto &pass = get(RasterPassTag{}, passID, *renderGraph);
    auto &subpassGraph = pass.subpassGraph;
    const auto subpassIndex = num_vertices(pass.subpassGraph);
    {
        auto id = addVertex(
            std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(),
            subpassGraph);
        CC_ENSURES(id == subpassIndex);
    }

    ComputeSubpass subpass(subpassIndex, renderGraph->get_allocator());

    auto subpassID = addVertex(
        ComputeSubpassTag{},
        std::forward_as_tuple(name),
        std::forward_as_tuple(layoutName.c_str()),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(std::move(subpass)),
        *renderGraph, passID);

    auto subpassLayoutID = locate(LayoutGraphData::null_vertex(), layoutName, *layoutGraph);
    CC_EXPECTS(subpassLayoutID != LayoutGraphData::null_vertex());

    auto *builder = ccnew NativeComputeSubpassBuilder(
        pipelineRuntime, renderGraph, subpassID, layoutGraph, subpassLayoutID);
    updateRasterPassConstants(pass.width, pass.height, *builder);

    return builder;
}

void NativeRasterPassBuilder::setViewport(const gfx::Viewport &viewport) {
    auto &pass = get(RasterPassTag{}, passID, *renderGraph);
    pass.viewport = viewport;
}

void NativeRasterPassBuilder::setMat4(const ccstd::string &name, const Mat4 &mat) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setMat4Impl(data, *layoutGraph, name, mat);
}

void NativeRasterPassBuilder::setQuaternion(const ccstd::string &name, const Quaternion &quat) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setQuaternionImpl(data, *layoutGraph, name, quat);
}

void NativeRasterPassBuilder::setColor(const ccstd::string &name, const gfx::Color &color) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setColorImpl(data, *layoutGraph, name, color);
}

void NativeRasterPassBuilder::setVec4(const ccstd::string &name, const Vec4 &vec) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setVec4Impl(data, *layoutGraph, name, vec);
}

void NativeRasterPassBuilder::setVec2(const ccstd::string &name, const Vec2 &vec) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setVec2Impl(data, *layoutGraph, name, vec);
}

void NativeRasterPassBuilder::setFloat(const ccstd::string &name, float v) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setFloatImpl(data, *layoutGraph, name, v);
}

void NativeRasterPassBuilder::setArrayBuffer(const ccstd::string &name, const ArrayBuffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setArrayBufferImpl(data, *layoutGraph, name, *buffer);
}

void NativeRasterPassBuilder::setBuffer(const ccstd::string &name, gfx::Buffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setBufferImpl(data, *layoutGraph, name, buffer);
}

void NativeRasterPassBuilder::setTexture(const ccstd::string &name, gfx::Texture *texture) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setTextureImpl(data, *layoutGraph, name, texture);
}

void NativeRasterPassBuilder::setReadWriteBuffer(const ccstd::string &name, gfx::Buffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setReadWriteBufferImpl(data, *layoutGraph, name, buffer);
}

void NativeRasterPassBuilder::setReadWriteTexture(const ccstd::string &name, gfx::Texture *texture) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setReadWriteTextureImpl(data, *layoutGraph, name, texture);
}

void NativeRasterPassBuilder::setSampler(const ccstd::string &name, gfx::Sampler *sampler) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setSamplerImpl(data, *layoutGraph, name, sampler);
}

void NativeRasterPassBuilder::setVersion(const ccstd::string &name, uint64_t version) {
    // noop
}

// NativeComputeQueue
ccstd::string NativeComputeQueueBuilder::getName() const {
    return std::string(get(RenderGraph::NameTag{}, *renderGraph, queueID));
}

void NativeComputeQueueBuilder::setName(const ccstd::string &name) {
    get(RenderGraph::NameTag{}, *renderGraph, queueID) = std::string_view{name};
}

void NativeComputeQueueBuilder::addDispatch(
    uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ,
    Material *material, uint32_t passID) {
    std::string_view name("Dispatch");
    addVertex(
        DispatchTag{},
        std::forward_as_tuple(name),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(
            material,
            passID,
            threadGroupCountX,
            threadGroupCountY,
            threadGroupCountZ),
        *renderGraph, passID);
}

void NativeComputeQueueBuilder::setMat4(const ccstd::string &name, const Mat4 &mat) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, queueID);
    setMat4Impl(data, *layoutGraph, name, mat);
}

void NativeComputeQueueBuilder::setQuaternion(const ccstd::string &name, const Quaternion &quat) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, queueID);
    setQuaternionImpl(data, *layoutGraph, name, quat);
}

void NativeComputeQueueBuilder::setColor(const ccstd::string &name, const gfx::Color &color) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, queueID);
    setColorImpl(data, *layoutGraph, name, color);
}

void NativeComputeQueueBuilder::setVec4(const ccstd::string &name, const Vec4 &vec) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, queueID);
    setVec4Impl(data, *layoutGraph, name, vec);
}

void NativeComputeQueueBuilder::setVec2(const ccstd::string &name, const Vec2 &vec) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, queueID);
    setVec2Impl(data, *layoutGraph, name, vec);
}

void NativeComputeQueueBuilder::setFloat(const ccstd::string &name, float v) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, queueID);
    setFloatImpl(data, *layoutGraph, name, v);
}

void NativeComputeQueueBuilder::setArrayBuffer(const ccstd::string &name, const ArrayBuffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, queueID);
    setArrayBufferImpl(data, *layoutGraph, name, *buffer);
}

void NativeComputeQueueBuilder::setBuffer(const ccstd::string &name, gfx::Buffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, queueID);
    setBufferImpl(data, *layoutGraph, name, buffer);
}

void NativeComputeQueueBuilder::setTexture(const ccstd::string &name, gfx::Texture *texture) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, queueID);
    setTextureImpl(data, *layoutGraph, name, texture);
}

void NativeComputeQueueBuilder::setReadWriteBuffer(const ccstd::string &name, gfx::Buffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, queueID);
    setReadWriteBufferImpl(data, *layoutGraph, name, buffer);
}

void NativeComputeQueueBuilder::setReadWriteTexture(const ccstd::string &name, gfx::Texture *texture) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, queueID);
    setReadWriteTextureImpl(data, *layoutGraph, name, texture);
}

void NativeComputeQueueBuilder::setSampler(const ccstd::string &name, gfx::Sampler *sampler) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, queueID);
    setSamplerImpl(data, *layoutGraph, name, sampler);
}

ccstd::string NativeComputePassBuilder::getName() const {
    return std::string(get(RenderGraph::NameTag{}, *renderGraph, passID));
}

void NativeComputePassBuilder::setName(const ccstd::string &name) {
    get(RenderGraph::NameTag{}, *renderGraph, passID) = std::string_view{name};
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void NativeComputePassBuilder::addTexture(const ccstd::string &name, const ccstd::string &slotName) {
}

void NativeComputePassBuilder::addStorageBuffer(const ccstd::string &name, AccessType accessType, const ccstd::string &slotName) {
}

void NativeComputePassBuilder::addStorageImage(const ccstd::string &name, AccessType accessType, const ccstd::string &slotName) {
}

void NativeComputePassBuilder::addComputeView(const ccstd::string &name, const ComputeView &view) {
    CC_EXPECTS(!name.empty());
    CC_EXPECTS(!view.name.empty());
    auto &pass = get(ComputeTag{}, passID, *renderGraph);
    auto iter = pass.computeViews.find(name.c_str());
    if (iter == pass.computeViews.end()) {
        bool added = false;
        std::tie(iter, added) = pass.computeViews.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(name.c_str()),
            std::forward_as_tuple());
        CC_ENSURES(added);
    }
    iter->second.emplace_back(view);
}

ComputeQueueBuilder *NativeComputePassBuilder::addQueue(const ccstd::string &layoutName) {
    CC_EXPECTS(layoutID != LayoutGraphData::null_vertex());

    auto phaseLayoutID = LayoutGraphData::null_vertex();
    if (!layoutName.empty()) {
        phaseLayoutID = locate(layoutID, layoutName, *layoutGraph);
        CC_ENSURES(phaseLayoutID != LayoutGraphData::null_vertex());
    }

    std::string_view name("Queue");
    auto queueID = addVertex(
        QueueTag{},
        std::forward_as_tuple(name),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(),
        std::forward_as_tuple(QueueHint::NONE, phaseLayoutID),
        *renderGraph, passID);

    return new NativeComputeQueueBuilder(renderGraph, queueID, layoutGraph);
}

void NativeComputePassBuilder::setMat4(const ccstd::string &name, const Mat4 &mat) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setMat4Impl(data, *layoutGraph, name, mat);
}

void NativeComputePassBuilder::setQuaternion(const ccstd::string &name, const Quaternion &quat) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setQuaternionImpl(data, *layoutGraph, name, quat);
}

void NativeComputePassBuilder::setColor(const ccstd::string &name, const gfx::Color &color) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setColorImpl(data, *layoutGraph, name, color);
}

void NativeComputePassBuilder::setVec4(const ccstd::string &name, const Vec4 &vec) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setVec4Impl(data, *layoutGraph, name, vec);
}

void NativeComputePassBuilder::setVec2(const ccstd::string &name, const Vec2 &vec) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setVec2Impl(data, *layoutGraph, name, vec);
}

void NativeComputePassBuilder::setFloat(const ccstd::string &name, float v) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setFloatImpl(data, *layoutGraph, name, v);
}

void NativeComputePassBuilder::setArrayBuffer(const ccstd::string &name, const ArrayBuffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setArrayBufferImpl(data, *layoutGraph, name, *buffer);
}

void NativeComputePassBuilder::setBuffer(const ccstd::string &name, gfx::Buffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setBufferImpl(data, *layoutGraph, name, buffer);
}

void NativeComputePassBuilder::setTexture(const ccstd::string &name, gfx::Texture *texture) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setTextureImpl(data, *layoutGraph, name, texture);
}

void NativeComputePassBuilder::setReadWriteBuffer(const ccstd::string &name, gfx::Buffer *buffer) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setReadWriteBufferImpl(data, *layoutGraph, name, buffer);
}

void NativeComputePassBuilder::setReadWriteTexture(const ccstd::string &name, gfx::Texture *texture) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setReadWriteTextureImpl(data, *layoutGraph, name, texture);
}

void NativeComputePassBuilder::setSampler(const ccstd::string &name, gfx::Sampler *sampler) {
    auto &data = get(RenderGraph::DataTag{}, *renderGraph, passID);
    setSamplerImpl(data, *layoutGraph, name, sampler);
}

ccstd::string NativeMovePassBuilder::getName() const {
    return std::string(get(RenderGraph::NameTag{}, *renderGraph, passID));
}

void NativeMovePassBuilder::setName(const ccstd::string &name) {
    get(RenderGraph::NameTag{}, *renderGraph, passID) = std::string_view{name};
}

void NativeMovePassBuilder::addPair(const MovePair &pair) {
    auto &movePass = get(MoveTag{}, passID, *renderGraph);
    movePass.movePairs.emplace_back(pair);
}

ccstd::string NativeCopyPassBuilder::getName() const {
    return std::string(get(RenderGraph::NameTag{}, *renderGraph, passID));
}

void NativeCopyPassBuilder::setName(const ccstd::string &name) {
    get(RenderGraph::NameTag{}, *renderGraph, passID) = std::string_view{name};
}

void NativeCopyPassBuilder::addPair(const CopyPair &pair) {
    auto &copyPass = get(CopyTag{}, passID, *renderGraph);
    copyPass.copyPairs.emplace_back(pair);
}

namespace {

const char *getName(const gfx::LoadOp &op) {
    switch (op) {
        case gfx::LoadOp::LOAD:
            return "LOAD";
        case gfx::LoadOp::CLEAR:
            return "CLEAR";
        case gfx::LoadOp::DISCARD:
            return "DISCARD";
        default:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}

const char *getName(const gfx::StoreOp &op) {
    switch (op) {
        case gfx::StoreOp::STORE:
            return "STORE";
        case gfx::StoreOp::DISCARD:
            return "DISCARD";
        default:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}

std::string getName(const gfx::ClearFlagBit &flags) {
    std::ostringstream oss;
    int count = 0;
    if (hasAnyFlags(flags, gfx::ClearFlagBit::COLOR)) {
        if (count++) {
            oss << "|";
        }
        oss << "COLOR";
    }
    if (hasAnyFlags(flags, gfx::ClearFlagBit::DEPTH)) {
        if (count++) {
            oss << "|";
        }
        oss << "DEPTH";
    }
    if (hasAnyFlags(flags, gfx::ClearFlagBit::STENCIL)) {
        if (count++) {
            oss << "|";
        }
        oss << "STENCIL";
    }
    if (flags == gfx::ClearFlagBit::NONE) {
        oss << "NONE";
    }
    return oss.str();
}

struct RenderGraphPrintVisitor : boost::dfs_visitor<> {
    void discover_vertex(
        RenderGraph::vertex_descriptor vertID,
        const AddressableView<RenderGraph> &gv) const {
        const auto &g = gv.mGraph;
        const auto &name = get(RenderGraph::NameTag{}, g, vertID);
        visitObject(
            vertID, gv.mGraph,
            [&](const RasterPass &pass) {
                OSS << "RasterPass \"" << name << "\" {\n";
                indent(space);
                for (const auto &[name, rasterView] : pass.rasterViews) {
                    OSS << "RasterView \"" << name << "\" {\n";
                    {
                        INDENT();
                        OSS << "slotName: \"" << rasterView.slotName << "\";\n";
                        OSS << "accessType: " << getName(rasterView.accessType) << ";\n";
                        OSS << "attachmentType: " << getName(rasterView.attachmentType) << ";\n";
                        OSS << "loadOp: " << getName(rasterView.loadOp) << ";\n";
                        OSS << "storeOp: " << getName(rasterView.storeOp) << ";\n";
                        OSS << "clearFlags: " << getName(rasterView.clearFlags) << ";\n";
                        const auto &c = rasterView.clearColor;
                        if (hasAnyFlags(rasterView.clearFlags, gfx::ClearFlagBit::COLOR)) {
                            OSS << "clearColor: [" << c.x << ", " << c.y << ", " << c.z << ", " << c.z << "];\n";
                        } else if (hasAnyFlags(rasterView.clearFlags, gfx::ClearFlagBit::DEPTH_STENCIL)) {
                            OSS << "clearColor: [" << c.x << ", " << c.y << "];\n";
                        }
                    }
                    OSS << "}\n";
                }
                for (const auto &[name, computeViews] : pass.computeViews) {
                    OSS << "ComputeViews \"" << name << "\" {\n";
                    {
                        INDENT();
                        for (const auto &computeView : computeViews) {
                            OSS << "ComputeView \"" << computeView.name << "\" {\n";
                            {
                                INDENT();
                                OSS << "accessType: " << getName(computeView.accessType) << ";\n";
                                OSS << "clearFlags: " << getName(computeView.clearFlags) << ";\n";
                                const auto &c = computeView.clearColor;
                                if (hasAnyFlags(computeView.clearFlags, gfx::ClearFlagBit::COLOR)) {
                                    OSS << "clearColor: [" << c.x << ", " << c.y << ", " << c.z << ", " << c.z << "];\n";
                                } else if (hasAnyFlags(computeView.clearFlags, gfx::ClearFlagBit::DEPTH_STENCIL)) {
                                    OSS << "clearColor: [" << c.x << ", " << c.y << "];\n";
                                }
                            }
                            OSS << "}\n";
                        }
                    }
                    OSS << "}\n";
                }
            },
            [&](const RasterSubpass &subpass) {
                std::ignore = subpass;
            },
            [&](const ComputeSubpass &subpass) {
                std::ignore = subpass;
            },
            [&](const ComputePass &pass) {
                OSS << "ComputePass \"" << name << "\" {\n";
                indent(space);
                for (const auto &[name, computeViews] : pass.computeViews) {
                    OSS << "ComputeViews \"" << name << "\" {\n";
                    {
                        INDENT();
                        for (const auto &computeView : computeViews) {
                            OSS << "ComputeView \"" << computeView.name << "\" {\n";
                            {
                                INDENT();
                                OSS << "accessType: " << getName(computeView.accessType) << ";\n";
                                OSS << "clearFlags: " << getName(computeView.clearFlags) << ";\n";
                                const auto &c = computeView.clearColor;
                                if (hasAnyFlags(computeView.clearFlags, gfx::ClearFlagBit::COLOR)) {
                                    OSS << "clearColor: [" << c.x << ", " << c.y << ", " << c.z << ", " << c.z << "];\n";
                                } else if (hasAnyFlags(computeView.clearFlags, gfx::ClearFlagBit::DEPTH_STENCIL)) {
                                    OSS << "clearColor: [" << c.x << ", " << c.y << "];\n";
                                }
                            }
                            OSS << "}\n";
                        }
                    }
                    OSS << "}\n";
                }
            },
            [&](const CopyPass &pass) {
                OSS << "CopyPass \"" << name << "\" {\n";
                for (const auto &pair : pass.copyPairs) {
                    INDENT();
                    OSS << "source: \"" << pair.source << "\", target: \"" << pair.target << "\"\n";
                }
                indent(space);
            },
            [&](const MovePass &pass) {
                OSS << "MovePass \"" << name << "\" {\n";
                for (const auto &pair : pass.movePairs) {
                    INDENT();
                    OSS << "source: \"" << pair.source << "\", target: \"" << pair.target << "\"\n";
                }
                indent(space);
            },
            [&](const RaytracePass &pass) {
                std::ignore = pass;
                OSS << "RaytracePass \"" << name << "\" {\n";
                indent(space);
            },
            [&](const RenderQueue &queue) {
                OSS << "Queue \"" << name << "\" {\n";
                {
                    INDENT();
                    OSS << "hint: " << getName(queue.hint) << ";\n";
                }
                indent(space);
            },
            [&](const SceneData &scene) {
                OSS << "Scene \"" << name << "\" {\n";
                {
                    INDENT();
                    // OSS << "name: \"" << scene.name << "\";\n";
                    OSS << "scenes: [";
                    int count = 0;
                    for (const auto &sceneName : scene.scenes) {
                        if (count++) {
                            oss << ", ";
                        }
                        oss << "\"" << sceneName << "\"";
                    }
                    oss << "];\n";
                    // OSS << "flags: " << getName(scene.flags) << ";\n";
                }
                indent(space);
            },
            [&](const Blit &blit) {
                std::ignore = blit;
                OSS << "Blit \"" << name << "\";\n";
            },
            [&](const Dispatch &dispatch) {
                OSS << "Dispatch \"" << name << "\", ["
                    << dispatch.threadGroupCountX << ", "
                    << dispatch.threadGroupCountY << ", "
                    << dispatch.threadGroupCountZ << "];\n";
            },
            [&](const ccstd::pmr::vector<ClearView> &clearViews) {
                OSS << "Clear \"" << name << "\" {\n";
                indent(space);
                for (const auto &view : clearViews) {
                    INDENT();
                    OSS << "\"" << view.slotName << "\" {\n";
                    {
                        INDENT();
                        OSS << "clearFlags: " << getName(view.clearFlags) << ";\n";
                        const auto &c = view.clearColor;
                        if (hasAnyFlags(view.clearFlags, gfx::ClearFlagBit::COLOR)) {
                            OSS << "clearColor: [" << c.x << ", " << c.y << ", " << c.z << ", " << c.z << "];\n";
                        } else if (hasAnyFlags(view.clearFlags, gfx::ClearFlagBit::DEPTH_STENCIL)) {
                            OSS << "clearColor: [" << c.x << ", " << c.y << "];\n";
                        }
                    }
                    OSS << "}\n";
                }
            },
            [&](const gfx::Viewport &vp) {
                OSS << "Viewport \"" << name << "\" ["
                    << "left: " << vp.left << ", "
                    << "top: " << vp.top << ", "
                    << "width: " << vp.width << ", "
                    << "height: " << vp.height << ", "
                    << "minDepth: " << vp.minDepth << ", "
                    << "maxDepth: " << vp.maxDepth << "]\n";
            });
    }

    void finish_vertex(
        RenderGraph::vertex_descriptor vertID,
        const AddressableView<RenderGraph> &gv) const {
        std::ignore = gv;
        visitObject(
            vertID, gv.mGraph,
            [&](const RasterPass &pass) {
                std::ignore = pass;
                unindent(space);
                OSS << "}\n";
            },
            [&](const RasterSubpass &subpass) {
                std::ignore = subpass;
            },
            [&](const ComputeSubpass &subpass) {
                std::ignore = subpass;
            },
            [&](const ComputePass &pass) {
                std::ignore = pass;
                unindent(space);
                OSS << "}\n";
            },
            [&](const CopyPass &pass) {
                std::ignore = pass;
                unindent(space);
                OSS << "}\n";
            },
            [&](const MovePass &pass) {
                std::ignore = pass;
                unindent(space);
                OSS << "}\n";
            },
            [&](const RaytracePass &pass) {
                std::ignore = pass;
                unindent(space);
                OSS << "}\n";
            },
            [&](const RenderQueue &queue) {
                std::ignore = queue;
                unindent(space);
                OSS << "}\n";
            },
            [&](const SceneData &scene) {
                std::ignore = scene;
                unindent(space);
                OSS << "}\n";
            },
            [&](const Blit &blit) {
            },
            [&](const Dispatch &dispatch) {
            },
            [&](const ccstd::pmr::vector<ClearView> &clear) {
                std::ignore = clear;
                unindent(space);
                OSS << "}\n";
            },
            [&](const gfx::Viewport &clear) {
            });
    }

    std::ostringstream &oss;
    ccstd::pmr::string &space;
};

} // namespace

ccstd::string RenderGraph::print(
    boost::container::pmr::memory_resource *scratch) const {
    const auto &rg = *this;
    std::ostringstream oss;
    ccstd::pmr::string space(scratch);
    oss << "\n";
    OSS << "RenderGraph {\n";
    {
        INDENT();
        RenderGraphPrintVisitor visitor{
            {}, oss, space};
        AddressableView<RenderGraph> graphView(rg);
        auto colors = rg.colors(scratch);
        boost::depth_first_search(graphView, visitor, get(colors, rg));
    }
    OSS << "}\n";
    return oss.str();
}

} // namespace render

} // namespace cc
