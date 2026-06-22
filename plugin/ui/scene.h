#pragma once
#include <filesystem>
#include <webgpu/webgpu.h>
#include "sharedHelper.h"
#include "utilityHelper.h"
#include "resourceManager.h"
#include "MyUniforms.h"
#include "geometryMath.h"
#include "circularFloor.h"
#include "cameraState.h"
#include "dragState.h"

#define WGPU_STR(s) WGPUStringView{s, sizeof(s) - 1}

namespace AnimatedLpg
{
    class Scene
    {
    public:
        Scene();
        ~Scene();
        void init(WGPUDevice device, WGPUQueue queue);
        void setSurface(WGPUSurface surface);
        void setShaderModule(WGPUShaderModule shaderModule);
        void setPipelineDesc(const WGPURenderPipelineDescriptor& pipelineDesc);
        bool createShader();
        void terminate();
        void reloadShader();
        void setUniforms(float time);
        void renderFrame(float currentTime);
        void ConfigureVertexLayout();
        void initializeScene();
        bool createPipeline();
        void initializeFloor();
        void updateDepthTexture(uint32_t width, uint32_t height);
        void updateViewMatrix();
        void onMouseButton(int button, bool isPressed, float xpos, float ypos);
        void onMouseMove(float xpos, float ypos);
        void onScroll(float deltaX, float deltaY);
        void setSurfaceSize(uint32_t width, uint32_t height) { mWidth = width; mHeight = height; }
        CameraState getCameraState() const { return mCameraState; }
        void setCameraState(const CameraState& s) { mCameraState = s; updateViewMatrix(); }
        void setSurfaceFormat(WGPUTextureFormat format) { mSurfaceFormat = format; }
        WGPUTextureView getDepthTextureView() const { return mDepthTextureView; }
        WGPUColorTargetState getColorTarget() const { return mColorTarget; }
        WGPUFragmentState getFragmentState() const { return mFragmentState; }
        WGPUBlendState getBlendState() const { return mBlendState; }
    private:
        void setItemBuffers(WGPUBuffer vertexBuffer, WGPUBuffer indexBuffer, uint32_t indexCount, uint32_t material, WGPURenderPassEncoder renderPass) const
        {
            if (vertexBuffer && indexBuffer && indexCount > 0)
            {
                const uint32_t offset = material * mUniformStride;
                wgpuRenderPassEncoderSetBindGroup(renderPass, 0, mBindGroup, 1, &offset);
                wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, vertexBuffer, 0, wgpuBufferGetSize(vertexBuffer));
                wgpuRenderPassEncoderSetIndexBuffer(renderPass, indexBuffer, WGPUIndexFormat_Uint16, 0, wgpuBufferGetSize(indexBuffer));
                wgpuRenderPassEncoderDrawIndexed(renderPass, indexCount, 1, 0, 0, 0);
            }
        }
        uint32_t                            mWidth               = {};
        uint32_t                            mHeight              = {};
        WGPUDevice                          mDevice              = nullptr;
        WGPUQueue                           mQueue               = nullptr;
        WGPURenderPipeline                  mPipeline            = {};
        WGPUSurface                         mSurface             = nullptr;
        WGPUTextureFormat                   mSurfaceFormat       = WGPUTextureFormat_Undefined;
        std::vector<std::filesystem::path>  mShaderPaths;
        std::filesystem::file_time_type     mLastShaderWriteTime;
        WGPUBuffer                          mUniformBuffer       = nullptr;
        WGPUTexture                         mDepthTexture        = nullptr;
        WGPUTextureView                     mDepthTextureView    = nullptr; //Revisit this
        WGPUTexture                         mMsaaTexture         = nullptr;
        WGPUTextureView                     mMsaaTextureView     = nullptr;
        uint32_t                            mUniformStride       = 0;
        WGPUBindGroup                       mBindGroup           = nullptr;
        std::array<WGPUVertexAttribute, 3>  mVertexAttribs       = {};
        std::vector<WGPUVertexBufferLayout> mVertexBufferLayouts = {};
        WGPUShaderModule                    mShaderModule        = {};
        WGPURenderPipelineDescriptor        mPipelineDesc        = {};
        WGPUColorTargetState                mColorTarget         = {};
        WGPUFragmentState                   mFragmentState       = {};
        WGPUBlendState                      mBlendState          = {};
        MyUniforms                          mUniforms            = {};
        //Floor
        WGPUBuffer                          mFloorVertexBuffer   = nullptr;
        WGPUBuffer                          mFloorIndexBuffer    = nullptr;
        uint32_t                            mFloorIndexCount     = 0;
        //Camera
        CameraState mCameraState;
        DragState mDrag;

    };
}

