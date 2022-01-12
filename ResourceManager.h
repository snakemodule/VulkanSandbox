#pragma once

#include <map>
#include <string>
#include <memory>

#include "AnimationKeys.h"

#include "SbTextureImage.h"
#include "Model.h"

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
    std::map<std::string, std::shared_ptr<SbTextureImage>> textures;
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

    std::shared_ptr<SbTextureImage> loadTexture2D(std::string name, std::string path)
    {
        assert(vkBase != nullptr);
        auto ptr = std::make_shared<SbTextureImage>(*vkBase, path);
        textures[name] = ptr;
        return ptr;
    }

    bool textureExists(std::string name)
    {
        return textures.find(name) != textures.end();
    }

};

