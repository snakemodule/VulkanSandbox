#pragma once

#include <map>
#include <string>
#include <memory>

#include "AnimationKeys.h"

#include "Texture.h"

#include "SbTextureImage.h"
#include "Model.h"

#include <gli.hpp>

#include "SbVulkanBase.h"
#include "VulkanHelperFunctions.h"
#include "SbBuffer.h"
#include "VulkanInitializers.hpp"

class ResourceManager
{
public:
    inline static ResourceManager& getInstance()
    {
        static ResourceManager instance; 
        return instance;
    }
private:
    ResourceManager() {}

public:
    ResourceManager(ResourceManager const&) = delete;
    void operator=(ResourceManager const&) = delete;

    SbVulkanBase* vkBase = nullptr;

	std::map<std::string, std::shared_ptr<AnimationKeys>> animations;
    std::map<std::string, std::shared_ptr<Texture>> textures;
    std::map<std::string, VkSampler> samplers;

    std::map<std::string, std::shared_ptr<StaticMesh>> staticMeshes;

    //std::shared_ptr<StaticMesh> loadStaticMesh(std::string path) //todo name
    //{
    //    auto ptr = std::make_shared<StaticMesh>(path);
    //    staticMeshes[path] = ptr;
    //    return ptr;
    //}

    std::shared_ptr<AnimationKeys> loadAnimation(std::string path) //todo name
    {
        auto ptr = std::make_shared<AnimationKeys>(path);
        animations[path] = ptr;
        return ptr;
    }

    std::shared_ptr<Texture> loadTexture2D(std::string name, std::string path, VkSampler sampler)
    {
        assert(vkBase != nullptr);

        auto texture = std::make_shared<Texture>();

        gli::texture2d tex2D = gli::texture2d(gli::load(path.c_str()));
        if (tex2D.empty()) { throw std::runtime_error("failed to load texture image!"); }
        auto& infoRef = texture->imageCreateInfo;
        infoRef.extent = VkExtent3D{
            static_cast<uint32_t>(tex2D.extent().x),
            static_cast<uint32_t>(tex2D.extent().y),
            0
        };
        texture->imageSize = tex2D.size();
        infoRef.mipLevels = static_cast<uint32_t>(tex2D.levels());
        infoRef.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        infoRef.imageType = VK_IMAGE_TYPE_2D;
        infoRef.extent.depth = 1;
        infoRef.arrayLayers = 1;
        infoRef.format = VK_FORMAT_BC2_UNORM_BLOCK;
        infoRef.tiling = VK_IMAGE_TILING_OPTIMAL;
        infoRef.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        infoRef.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        infoRef.samples = VK_SAMPLE_COUNT_1_BIT;
        infoRef.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        texture->image = vkBase->getDevice().createImage(infoRef);

        vks::helper::createImageMemory(vkBase->getPhysicalDevice(), vkBase->getDevice(), 
            texture->image, &texture->memory);        
        vkBase->commandPool->transitionImageLayout(texture->image, VK_FORMAT_R8G8B8A8_UNORM, 
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture->imageCreateInfo.mipLevels);

        SbBuffer stagingBuffer = SbBuffer(*vkBase, texture->imageSize, vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        stagingBuffer.MapAndFill(vkBase->getDevice(), tex2D.data(), static_cast<size_t>((VkDeviceSize)texture->imageSize));

        VkCommandBuffer commandBuffer = vkBase->commandPool->beginSingleTimeCommands();        
        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = texture->imageCreateInfo.mipLevels;
        subresourceRange.layerCount = 1;

        vks::helper::setImageLayout(
            commandBuffer,
            texture->image,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            subresourceRange);

        vks::helper::copyBufferToImage(commandBuffer, tex2D, texture->imageCreateInfo.mipLevels, 
            stagingBuffer.buffer, texture->image);              

        vks::helper::setImageLayout(
            commandBuffer,
            texture->image,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            subresourceRange);

        vkBase->commandPool->endSingleTimeCommands(commandBuffer);

        stagingBuffer.Destroy(vkBase->getDevice());

        texture->imageView = vks::helper::createImageView(vkBase->logicalDevice->device, texture->image,
            texture->imageCreateInfo.format, VK_IMAGE_ASPECT_COLOR_BIT, texture->imageCreateInfo.mipLevels);


        texture->descriptorInfo = vks::initializers::descriptorImageInfo(sampler, 
            texture->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);

        
        textures[name] = texture;
        return texture;
    }

    VkSampler createTextureSampler(std::string name, vk::SamplerCreateInfo samplerCreateInfo)
    {
        auto sampler = vkBase->getDevice().createSampler(samplerCreateInfo);
        samplers[name] = sampler;
        return sampler;
    }

    bool textureExists(std::string name)
    {
        return textures.find(name) != textures.end();
    }

    

};

