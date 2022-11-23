/****************************************************************************
Copyright (c) 2019-2022 Xiamen Yaji Software Co., Ltd.

http://www.cocos.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated engine source code (the "Software"), a limited,
worldwide, royalty-free, non-assignable, revocable and non-exclusive license
to use Cocos Creator solely to develop games on your target platforms. You shall
not use Cocos Creator software for developing other software or tools that's
used for developing games. You are not granted to publish, distribute,
sublicense, and/or sell copies of Cocos Creator.

The software or tools in this License Agreement are licensed, not sold.
Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "gfx-metal/MTLTransientPool.h"
#include "gfx-metal/MTLDevice.h"
#include "gfx-metal/MTLBuffer.h"
#include "gfx-metal/MTLTexture.h"
#include "gfx-metal/MTLDevice.h"

namespace cc {
namespace gfx {
static MTLRenderStages getRenderStage(AccessFlags accessFlag) {
    static const AccessFlags VertexStageFlags =
        AccessFlagBit::INDIRECT_BUFFER |
        AccessFlagBit::INDEX_BUFFER |
        AccessFlagBit::VERTEX_BUFFER |
        AccessFlagBit::VERTEX_SHADER_READ_UNIFORM_BUFFER |
        AccessFlagBit::VERTEX_SHADER_READ_TEXTURE |
        AccessFlagBit::VERTEX_SHADER_READ_OTHER |
        AccessFlagBit::VERTEX_SHADER_WRITE;

    static const AccessFlags FragmentStageFlags =
        AccessFlagBit::FRAGMENT_SHADER_READ_UNIFORM_BUFFER |
        AccessFlagBit::FRAGMENT_SHADER_READ_TEXTURE |
        AccessFlagBit::FRAGMENT_SHADER_READ_COLOR_INPUT_ATTACHMENT |
        AccessFlagBit::FRAGMENT_SHADER_READ_DEPTH_STENCIL_INPUT_ATTACHMENT |
        AccessFlagBit::FRAGMENT_SHADER_READ_OTHER |
        AccessFlagBit::COLOR_ATTACHMENT_READ |
        AccessFlagBit::DEPTH_STENCIL_ATTACHMENT_READ |
        AccessFlagBit::FRAGMENT_SHADER_WRITE |
        AccessFlagBit::COLOR_ATTACHMENT_WRITE |
        AccessFlagBit::DEPTH_STENCIL_ATTACHMENT_WRITE;

    if ((accessFlag & VertexStageFlags) != AccessFlagBit::NONE) {
        return MTLRenderStageVertex;
    }
    if ((accessFlag & FragmentStageFlags) != AccessFlagBit::NONE) {
        return MTLRenderStageFragment;
    }
    return MTLRenderStageVertex;
}

CCMTLTransientPool::CCMTLTransientPool() {
    _typedID = generateObjectID<decltype(this)>();
}

void CCMTLTransientPool::init(const TransientPoolInfo &info) {
    AllocatorInfo allocatorInfo = {};
    allocatorInfo.chunkSize = info.blockSize;
    _allocator = std::make_unique<Allocator>(allocatorInfo);
    _allocator->setMemoryImpl(this);
    _context = std::make_unique<AliasingContext>();
}

bool CCMTLTransientPool::allocate() {
    auto device = (id<MTLDevice>)CCMTLDevice::getInstance()->getMTLDevice();

    MTLHeapDescriptor* heapDescriptor = [[MTLHeapDescriptor alloc] init];
    heapDescriptor.size = _info.blockSize;
    heapDescriptor.storageMode = MTLStorageModePrivate;  // no CPU and GPU coherency
    heapDescriptor.cpuCacheMode = MTLCPUCacheModeDefaultCache;

    if (@available(ios 13, macos 10.15, *)) {
        heapDescriptor.type = MTLHeapTypePlacement;
        heapDescriptor.hazardTrackingMode = MTLHazardTrackingModeTracked;
    }

    id<MTLHeap> heap = [device newHeapWithDescriptor:heapDescriptor];
    [heapDescriptor release];

    if (heap == nil) {
        return false;
    }
    _heaps.emplace_back(heap);
    return true;
}

void CCMTLTransientPool::free(uint32_t index) {
    [_heaps[index] release];
    _heaps[index] = nil;
}

void CCMTLTransientPool::doInit(const TransientPoolInfo &info) {
    init(info);
}

void CCMTLTransientPool::doInitBuffer(Buffer *buffer, PassScope scope, AccessFlags accessFlag) {
    auto *mtlBuffer = static_cast<CCMTLBuffer *>(buffer);

    MTLSizeAndAlign sizeAndAlign = mtlBuffer->getSizeAndAlign();
    auto *memoryView = _allocator->allocate(sizeAndAlign.size, sizeAndAlign.align);

    mtlBuffer->initFromHeap(_heaps[memoryView->memoryIndex], sizeAndAlign.size, memoryView->offset);
    mtlBuffer->setAllocation(memoryView);

    auto &res = _resources[buffer->getObjectID()];
    res.resource.object = buffer;
    res.first = scope;
    res.firstAccess = accessFlag;
}

void CCMTLTransientPool::doResetBuffer(Buffer *buffer, PassScope scope, AccessFlags accessFlag) {
    auto *mtlBuffer = static_cast<CCMTLBuffer *>(buffer);
    auto *memoryView = mtlBuffer->getAllocation();

    auto iter = _resources.find(buffer->getObjectID());
    if (iter != _resources.end()) {
        iter->second.last = scope;
        iter->second.lastAccess = accessFlag;
        _context->record({iter->second,
                          memoryView->memoryIndex,
                          memoryView->offset,
                          memoryView->offset + memoryView->size - 1
        });
    }
    _allocator->free(mtlBuffer->getAllocation());
    mtlBuffer->setAllocation(nullptr);
}

void CCMTLTransientPool::doInitTexture(Texture *texture, PassScope scope, AccessFlags accessFlag) {
    auto *mtlTexture = static_cast<CCMTLTexture *>(texture);

    MTLSizeAndAlign sizeAndAlign = mtlTexture->getSizeAndAlign();
    auto *memoryView = _allocator->allocate(sizeAndAlign.size, sizeAndAlign.align);

    mtlTexture->initFromHeap(_heaps[memoryView->memoryIndex], memoryView->offset);
    mtlTexture->setAllocation(memoryView);

    auto &res = _resources[texture->getObjectID()];
    res.resource.object = texture;
    res.first = scope;
    res.firstAccess = accessFlag;
}

void CCMTLTransientPool::doResetTexture(Texture *texture, PassScope scope, AccessFlags accessFlag) {
    auto *mtlTexture = static_cast<CCMTLTexture *>(texture);
    auto *memoryView = mtlTexture->getAllocation();

    auto iter = _resources.find(texture->getObjectID());
    if (iter != _resources.end()) {
        iter->second.last = scope;
        iter->second.lastAccess = accessFlag;
        _context->record({iter->second,
                          memoryView->memoryIndex,
                          memoryView->offset,
                          memoryView->offset + memoryView->size - 1
        });
    }
    _allocator->free(mtlTexture->getAllocation());
    mtlTexture->setAllocation(nullptr);
}

void CCMTLTransientPool::frontBarrier(PassScope scope, CommandBuffer *cmdBuffer) {
    auto *mtlCmdBuffer = static_cast<CCMTLCommandBuffer *>(cmdBuffer);

    const auto &aliasingData = _context->getAliasingData();
    auto iter = aliasingData.find(scope);
    if (iter != aliasingData.end()) {
        for (const auto &aliasingInfo : iter->second) {
            id<MTLFence> fence = allocateFence();
            mtlCmdBuffer->updateFence(fence, getRenderStage(aliasingInfo.beforeAccess));
            _fencesToWait[aliasingInfo.nextScope].emplace_back(fence, aliasingInfo.afterAccess);
        }
    }

    auto waitFences = _fencesToWait.find(scope);
    if (waitFences != _fencesToWait.end()) {
        for (auto &[fence, access] : waitFences->second) {
            mtlCmdBuffer->waitFence(fence, getRenderStage(access));
            freeFence(fence);
        }
    }
}

void CCMTLTransientPool::rearBarrier(PassScope scope, CommandBuffer *cmdBuffer) {
}

id<MTLFence> CCMTLTransientPool::allocateFence() {
    if (!_freeList.empty()) {
        auto back = _freeList.back();
        _freeList.pop_back();
        return back;
    }
    auto fence = [static_cast<id<MTLDevice>>(CCMTLDevice::getInstance()->getMTLDevice()) newFence];
    return fence;
}

void CCMTLTransientPool::freeFence(id<MTLFence> fence) {
    _freeList.emplace_back(fence);
}

void CCMTLTransientPool::doBeginFrame() {
    _context->clear();
    _resources.clear();
    _fencesToWait.clear();
}

void CCMTLTransientPool::doEndFrame() {

}


} // namespace gfx
} // namespace cc
