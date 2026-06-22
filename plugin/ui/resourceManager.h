#pragma once
#include <vector>
#include <filesystem>
#include <webgpu/webgpu.h>

namespace AnimatedLpg
{
    class ResourceManager {
    public:
        static bool loadGeometry(const std::filesystem::path& path,
                                       std::vector<float>& pointData,
                                       std::vector<uint16_t>& indexData,
                                       int dimensions);
        /**
     * Create a shader module for a given WebGPU `device` from a WGSL shader source
     * loaded from file `path`.
     */
        static WGPUShaderModule loadShaderModule(
            const std::filesystem::path& path,
            WGPUDevice device
        );
        static WGPUShaderModule loadShaderModules(const std::vector<std::filesystem::path>& paths,
                                                        WGPUDevice device);

    private:
        std::string                    mShaderSource;

    };
}


