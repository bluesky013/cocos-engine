//
// Created by Zach Lee on 2023/7/16.
//

#include "GPUTimeQuery.h"
#include "gfx-base/GFXDevice.h"

namespace cc::render {

void GPUTimeQuery::clear() {
    _queries.clear();
}

void GPUTimeQuery::registerTimeStamp(const std::string &name) {
    _queries.emplace_back(name);
}

void GPUTimeQuery::updateQuery() {
    auto count = static_cast<uint32_t>(_queries.size());
    if (count <= _capacity) {
        return;
    }

    _capacity = count * 2;

    auto *device = gfx::Device::getInstance();
    gfx::QueryPoolInfo poolInfo = {};
    poolInfo.type = gfx::QueryType::TIMESTAMP;
    poolInfo.maxQueryObjects = _capacity;
    _queryPool = device->createQueryPool(poolInfo);

    gfx::BufferInfo bufferInfo = {};
    bufferInfo.usage = gfx::BufferUsageBit::TRANSFER_DST;
    bufferInfo.memUsage = gfx::MemoryUsageBit::HOST | gfx::MemoryUsageBit::DEVICE;
    bufferInfo.size = _capacity * sizeof(uint64_t);
    bufferInfo.stride = sizeof(uint64_t);
    _readBackBuffer = device->createBuffer(bufferInfo);
}

} // namespace cc::render