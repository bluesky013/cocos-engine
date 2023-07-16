//
// Created by Zach Lee on 2023/7/16.
//

#pragma once

#include <string>
#include "base/Ptr.h"
#include "base/std/container/vector.h"
#include "gfx-base/GFXQueryPool.h"

namespace cc::render {

class GPUTimeQuery {
public:
    GPUTimeQuery() = default;
    ~GPUTimeQuery() = default;

    void clear();
    void registerTimeStamp(const std::string &name);
    void updateQuery();

private:
    uint32_t _capacity = 0;
    ccstd::vector<std::string> _queries;
    IntrusivePtr<gfx::QueryPool> _queryPool;
    IntrusivePtr<gfx::Buffer> _readBackBuffer;
};


} // namespace cc::render