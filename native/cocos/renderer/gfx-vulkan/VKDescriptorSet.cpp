/****************************************************************************
 Copyright (c) 2020-2022 Xiamen Yaji Software Co., Ltd.

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

#include "VKDescriptorSet.h"
#include "VKBuffer.h"
#include "VKCommands.h"
#include "VKDescriptorSetLayout.h"
#include "VKDevice.h"
#include "VKPipelineLayout.h"
#include "VKShader.h"
#include "VKTexture.h"
#include "states/VKSampler.h"

namespace cc {
namespace gfx {

static void updateDescriptorBufferInfo(const CCVKGPUBufferView *buffer, VkDescriptorBufferInfo *descriptor, uint32_t index) {
    descriptor->buffer = buffer->gpuBuffer->vkBuffer;
    descriptor->offset = buffer->getStartOffset(index);
    descriptor->range = buffer->range;
}

static void updateDescriptorImageInfo(const CCVKGPUTextureView *texture, VkDescriptorImageInfo *descriptor) {
    descriptor->imageView = texture->vkImageView;
    if (hasFlag(texture->gpuTexture->flags, TextureFlagBit::GENERAL_LAYOUT)) {
        descriptor->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    } else {
        if (hasFlag(texture->gpuTexture->usage, TextureUsage::DEPTH_STENCIL_ATTACHMENT)) {
            descriptor->imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        } else {
            descriptor->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
    }
}

static void updateDescriptorImageInfo(const CCVKGPUSampler *sampler, VkDescriptorImageInfo *descriptor) {
    descriptor->sampler = sampler->vkSampler;
}

CCVKDescriptorSet::CCVKDescriptorSet() {
    _typedID = generateObjectID<decltype(this)>();
}

CCVKDescriptorSet::~CCVKDescriptorSet() {
    destroy();
}

void CCVKDescriptorSet::createGPUDescriptorSet() {

}

void CCVKDescriptorSet::updateGPUDescriptorSet() {

}

void CCVKDescriptorSet::doInit(const DescriptorSetInfo & /*info*/) {
    CCVKGPUDescriptorSetLayout *gpuDescriptorSetLayout = static_cast<CCVKDescriptorSetLayout *>(_layout)->gpuDescriptorSetLayout();
    uint32_t bindingCount = utils::toUint(gpuDescriptorSetLayout->bindings.size());
    uint32_t descriptorCount = gpuDescriptorSetLayout->descriptorCount;

    _gpuDescriptorSet = ccnew CCVKGPUDescriptorSet;
    _gpuDescriptorSet->gpuDescriptors.resize(descriptorCount, {});
    _gpuDescriptorSet->layoutID = gpuDescriptorSetLayout->id;
    _gpuDescriptorSet->baseOffsets.resize(gpuDescriptorSetLayout->dynamicBindings.size());

    for (size_t i = 0U, k = 0U; i < bindingCount; ++i) {
        const DescriptorSetLayoutBinding &binding = gpuDescriptorSetLayout->bindings[i];
        for (uint32_t j = 0; j < binding.count; ++j, ++k) {
            CCVKGPUDescriptor &gpuDescriptor = _gpuDescriptorSet->gpuDescriptors[k];
            gpuDescriptor.type = binding.descriptorType;
            switch (binding.descriptorType) {
                case DescriptorType::UNIFORM_BUFFER:
                case DescriptorType::DYNAMIC_UNIFORM_BUFFER:
                    if (hasFlag(binding.stageFlags, ShaderStageFlags::COMPUTE)) gpuDescriptor.accessTypes.push_back(THSVS_ACCESS_COMPUTE_SHADER_READ_UNIFORM_BUFFER);
                    if (hasFlag(binding.stageFlags, ShaderStageFlags::VERTEX)) gpuDescriptor.accessTypes.push_back(THSVS_ACCESS_VERTEX_SHADER_READ_UNIFORM_BUFFER);
                    if (hasFlag(binding.stageFlags, ShaderStageFlags::FRAGMENT)) gpuDescriptor.accessTypes.push_back(THSVS_ACCESS_FRAGMENT_SHADER_READ_UNIFORM_BUFFER);
                    break;
                case DescriptorType::STORAGE_BUFFER:
                case DescriptorType::DYNAMIC_STORAGE_BUFFER:
                case DescriptorType::STORAGE_IMAGE:
                    // write accesses should be handled manually
                    if (hasFlag(binding.stageFlags, ShaderStageFlags::COMPUTE)) gpuDescriptor.accessTypes.push_back(THSVS_ACCESS_COMPUTE_SHADER_READ_OTHER);
                    if (hasFlag(binding.stageFlags, ShaderStageFlags::VERTEX)) gpuDescriptor.accessTypes.push_back(THSVS_ACCESS_VERTEX_SHADER_READ_OTHER);
                    if (hasFlag(binding.stageFlags, ShaderStageFlags::FRAGMENT)) gpuDescriptor.accessTypes.push_back(THSVS_ACCESS_FRAGMENT_SHADER_READ_OTHER);
                    break;
                case DescriptorType::SAMPLER_TEXTURE:
                case DescriptorType::TEXTURE:
                    if (hasFlag(binding.stageFlags, ShaderStageFlags::COMPUTE)) gpuDescriptor.accessTypes.push_back(THSVS_ACCESS_COMPUTE_SHADER_READ_SAMPLED_IMAGE_OR_UNIFORM_TEXEL_BUFFER);
                    if (hasFlag(binding.stageFlags, ShaderStageFlags::VERTEX)) gpuDescriptor.accessTypes.push_back(THSVS_ACCESS_VERTEX_SHADER_READ_SAMPLED_IMAGE_OR_UNIFORM_TEXEL_BUFFER);
                    if (hasFlag(binding.stageFlags, ShaderStageFlags::FRAGMENT)) gpuDescriptor.accessTypes.push_back(THSVS_ACCESS_FRAGMENT_SHADER_READ_SAMPLED_IMAGE_OR_UNIFORM_TEXEL_BUFFER);
                    break;
                case DescriptorType::INPUT_ATTACHMENT:
                    gpuDescriptor.accessTypes.push_back(THSVS_ACCESS_FRAGMENT_SHADER_READ_COLOR_INPUT_ATTACHMENT);
                    break;
                case DescriptorType::SAMPLER:
                default:
                    break;
            }
        }
    }

    CCVKGPUDevice *gpuDevice = CCVKDevice::getInstance()->gpuDevice();
    _gpuDescriptorSet->gpuLayout = gpuDescriptorSetLayout;
    _gpuDescriptorSet->instances.resize(gpuDevice->backBufferCount);

    for (uint32_t t = 0U; t < gpuDevice->backBufferCount; ++t) {
        CCVKGPUDescriptorSet::Instance &instance = _gpuDescriptorSet->instances[t];
        instance.vkDescriptorSet = gpuDevice->getDescriptorSetPool(_gpuDescriptorSet->layoutID)->request();
        instance.descriptorInfos.resize(descriptorCount, {});

        for (uint32_t i = 0U, k = 0U; i < bindingCount; ++i) {
            const DescriptorSetLayoutBinding &binding = gpuDescriptorSetLayout->bindings[i];
            for (uint32_t j = 0; j < binding.count; ++j, ++k) {
                if (hasFlag(DESCRIPTOR_BUFFER_TYPE, binding.descriptorType)) {
                    instance.descriptorInfos[k].buffer.buffer = gpuDevice->defaultBuffer->vkBuffer;
                    instance.descriptorInfos[k].buffer.offset = gpuDevice->defaultBuffer->getStartOffset(t);
                    instance.descriptorInfos[k].buffer.range = gpuDevice->defaultBuffer->size;
                } else if (hasFlag(DESCRIPTOR_TEXTURE_TYPE, binding.descriptorType)) {
                    instance.descriptorInfos[k].image.sampler = gpuDevice->defaultSampler->vkSampler;
                    instance.descriptorInfos[k].image.imageView = gpuDevice->defaultTextureView->vkImageView;
                    instance.descriptorInfos[k].image.imageLayout = hasFlag(binding.descriptorType, DescriptorType::STORAGE_IMAGE)
                                                                        ? VK_IMAGE_LAYOUT_GENERAL
                                                                        : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                }
            }
        }

        if (!gpuDevice->useDescriptorUpdateTemplate) {
            ccstd::vector<VkWriteDescriptorSet> &entries = instance.descriptorUpdateEntries;
            entries.resize(descriptorCount, {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET});

            for (uint32_t i = 0U, j = 0U; i < bindingCount; i++) {
                const VkDescriptorSetLayoutBinding &descriptor = gpuDescriptorSetLayout->vkBindings[i];
                for (uint32_t k = 0U; k < descriptor.descriptorCount; k++, j++) {
                    entries[j].dstSet = instance.vkDescriptorSet;
                    entries[j].dstBinding = descriptor.binding;
                    entries[j].dstArrayElement = k;
                    entries[j].descriptorCount = 1; // better not to assume that the descriptor infos would be contiguous
                    entries[j].descriptorType = descriptor.descriptorType;
                    switch (entries[j].descriptorType) {
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                            entries[j].pBufferInfo = &instance.descriptorInfos[j].buffer;
                            break;
                        case VK_DESCRIPTOR_TYPE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                            entries[j].pImageInfo = &instance.descriptorInfos[j].image;
                            break;
                        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                            entries[j].pTexelBufferView = &instance.descriptorInfos[j].texelBufferView;
                            break;
                        default: break;
                    }
                }
            }
        }
    }
}

void CCVKDescriptorSet::doDestroy() {
    _gpuDescriptorSet = nullptr;
}

void CCVKDescriptorSet::updateBufferDescriptor(uint32_t index, CCVKGPUDescriptor &binding) {
    if (_buffers[index].ptr) {
        CCVKGPUDescriptorHub *descriptorHub = CCVKDevice::getInstance()->gpuDescriptorHub();
        CCVKGPUBufferView *bufferView = static_cast<CCVKBuffer *>(_buffers[index].ptr)->gpuBufferView();
        for (uint32_t i = 0; i < _gpuDescriptorSet->instances.size(); ++i) {
            VkDescriptorBufferInfo &descriptorInfo = _gpuDescriptorSet->instances[i].descriptorInfos[index].buffer;

            if (!hasFlag(_buffers[index].ptr->getFlags(), BufferFlagBit::DISABLE_RESIZE)) {
                if (binding.gpuBufferView) {
                    descriptorHub->disengage(_gpuDescriptorSet, binding.gpuBufferView);
                }
                if (bufferView) {
                    descriptorHub->connect(_gpuDescriptorSet, bufferView);
                }
            }
            updateDescriptorBufferInfo(bufferView, &descriptorInfo, i);
        }
    }
}

void CCVKDescriptorSet::updateImageDescriptor(uint32_t index, CCVKGPUDescriptor &binding) {
    if (_textures[index].ptr) {
        CCVKGPUDescriptorHub *descriptorHub = CCVKDevice::getInstance()->gpuDescriptorHub();
        CCVKGPUBarrierManager *layoutMgr = CCVKDevice::getInstance()->gpuBarrierManager();

        CCVKGPUTextureView *textureView = static_cast<CCVKTexture *>(_textures[index].ptr)->gpuTextureView();
        for (auto &instance : _gpuDescriptorSet->instances) {
            VkDescriptorImageInfo &descriptorInfo = instance.descriptorInfos[index].image;

            if (!hasFlag(_textures[index].ptr->getInfo().flags, TextureFlagBit::DISABLE_RESIZE)) {
                if (binding.gpuTextureView) {
                    descriptorHub->disengage(_gpuDescriptorSet, binding.gpuTextureView);
                }
                if (textureView) {
                    descriptorHub->connect(_gpuDescriptorSet, textureView);
                }
            }

            updateDescriptorImageInfo(textureView, &descriptorInfo);

            if (textureView) {
                layoutMgr->checkIn(textureView->gpuTexture, binding.accessTypes.data(), utils::toUint(binding.accessTypes.size()));
            }
        }
        binding.gpuTextureView = textureView;
    }
}

void CCVKDescriptorSet::updateSamplerDescriptor(uint32_t index, CCVKGPUDescriptor &binding) {
    if (_samplers[index].ptr) {
        CCVKGPUSampler *sampler = static_cast<CCVKSampler *>(_samplers[index].ptr)->gpuSampler();
        for (auto &instance : _gpuDescriptorSet->instances) {
            VkDescriptorImageInfo &descriptorInfo = instance.descriptorInfos[index].image;
            updateDescriptorImageInfo(sampler, &descriptorInfo);
        }
        binding.gpuSampler = sampler;
    }
}

void CCVKDescriptorSet::update() {
    if (_isDirty && _gpuDescriptorSet) {
        uint32_t descriptorCount = utils::toUint(_gpuDescriptorSet->gpuDescriptors.size());
        for (size_t i = 0U; i < descriptorCount; i++) {
            CCVKGPUDescriptor &binding = _gpuDescriptorSet->gpuDescriptors[i];

            if (hasFlag(DESCRIPTOR_BUFFER_TYPE, binding.type)) {
                updateBufferDescriptor(i, binding);
            } else if (hasFlag(DESCRIPTOR_TEXTURE_TYPE, binding.type)) {
                updateImageDescriptor(i, binding);
                updateSamplerDescriptor(i, binding);
            }
        }
        CCVKDevice::getInstance()->gpuDescriptorSetHub()->record(_gpuDescriptorSet);
        _isDirty = false;
    }
}

void CCVKDescriptorSet::forceUpdate() {
    _isDirty = true;
    update();
}

void CCVKGPUDescriptorSet::shutdown() {
    CCVKDevice *device = CCVKDevice::getInstance();
    CCVKGPUDescriptorHub *descriptorHub = CCVKDevice::getInstance()->gpuDescriptorHub();
    uint32_t instanceCount = utils::toUint(instances.size());

    for (uint32_t t = 0U; t < instanceCount; ++t) {
        CCVKGPUDescriptorSet::Instance &instance = instances[t];

        for (uint32_t i = 0U; i < gpuDescriptors.size(); i++) {
            CCVKGPUDescriptor &binding = gpuDescriptors[i];

            CCVKDescriptorInfo &descriptorInfo = instance.descriptorInfos[i];
            if (binding.gpuBufferView) {
                descriptorHub->disengage(this, binding.gpuBufferView);
            }
            if (binding.gpuTextureView) {
                descriptorHub->disengage(this, binding.gpuTextureView);
            }
        }

        if (instance.vkDescriptorSet) {
            device->gpuRecycleBin()->collect(layoutID, instance.vkDescriptorSet);
        }
    }

    CCVKDevice::getInstance()->gpuDescriptorSetHub()->erase(this);
}

void CCVKGPUDescriptorSet::update(const CCVKGPUBufferView *oldView, const CCVKGPUBufferView *newView) {
    CCVKGPUDescriptorHub *descriptorHub = CCVKDevice::getInstance()->gpuDescriptorHub();
    uint32_t instanceCount = utils::toUint(instances.size());

    for (size_t i = 0U; i < gpuDescriptors.size(); i++) {
        CCVKGPUDescriptor &binding = gpuDescriptors[i];
        if (hasFlag(DESCRIPTOR_BUFFER_TYPE, binding.type) && (binding.gpuBufferView == oldView)) {

            for (uint32_t t = 0U; t < instanceCount; ++t) {
                CCVKDescriptorInfo &descriptorInfo = instances[t].descriptorInfos[i];

                if (newView != nullptr) {
                    descriptorHub->connect(this, newView);
                    updateDescriptorBufferInfo(newView, &descriptorInfo.buffer, t);
                }

            }
            binding.gpuBufferView = newView;
        }
    }
    CCVKDevice::getInstance()->gpuDescriptorSetHub()->record(this);
}

void CCVKGPUDescriptorSet::update(const CCVKGPUTextureView *oldView, const CCVKGPUTextureView *newView) {
    CCVKGPUDescriptorHub *descriptorHub = CCVKDevice::getInstance()->gpuDescriptorHub();
    uint32_t instanceCount = utils::toUint(instances.size());

    for (size_t i = 0U; i < gpuDescriptors.size(); i++) {
        CCVKGPUDescriptor &binding = gpuDescriptors[i];
        if (hasFlag(DESCRIPTOR_TEXTURE_TYPE, binding.type) && (binding.gpuTextureView == oldView)) {
            for (uint32_t t = 0U; t < instanceCount; ++t) {
                CCVKDescriptorInfo &descriptorInfo = instances[t].descriptorInfos[i];

                if (newView != nullptr) {
                    descriptorHub->connect(this, newView);
                    updateDescriptorImageInfo(newView, &descriptorInfo.image);
                }
            }
            binding.gpuTextureView = newView;
        }
    }
    CCVKDevice::getInstance()->gpuDescriptorSetHub()->record(this);
}

} // namespace gfx
} // namespace cc
