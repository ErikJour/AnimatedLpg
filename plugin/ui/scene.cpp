//
// Created by Erik Jourgensen on 6/20/26.
//

#include "scene.h"
#include <cmath>

#include "sphereGeometry.h"

namespace {

void buildLookAt(float out[16],
                        const float ex, float ey, float ez,
                        float tx, float ty, float tz)
{
    float fx = tx-ex, fy = ty-ey, fz = tz-ez;
    const float fl = sqrtf(fx*fx + fy*fy + fz*fz);
    fx/=fl; fy/=fl; fz/=fl;

    float rx = fz, ry = 0.0f, rz = -fx;
    const float rl = sqrtf(rx*rx + ry*ry + rz*rz);
    rx/=rl; ry/=rl; rz/=rl;

    const float ux = fy*rz - fz*ry;
    const float uy = fz*rx - fx*rz;
    const float uz = fx*ry - fy*rx;

    out[0]=rx;   out[4]=ux;   out[8] =-fx;  out[12]=-(rx*ex+ry*ey+rz*ez);
    out[1]=ry;   out[5]=uy;   out[9] =-fy;  out[13]=-(ux*ex+uy*ey+uz*ez);
    out[2]=rz;   out[6]=uz;   out[10]=-fz;  out[14]=  fx*ex+fy*ey+fz*ez;
    out[3]=0.0f; out[7]=0.0f; out[11]=0.0f; out[15]=1.0f;
}

void buildPerspective(float out[16], float fovY, float aspect, float zNear, float zFar)
{
    const float f = 1.0f / tanf(fovY * 0.5f);
    std::memset(out, 0, 64);
    out[0]  = f / aspect;
    out[5]  = f;
    out[10] = zFar / (zNear - zFar);
    out[11] = -1.0f;
    out[14] = (zNear * zFar) / (zNear - zFar);
}

void mulMat4(float out[16], const float a[16], const float b[16])
{
    for (int col = 0; col < 4; ++col)
        for (int row = 0; row < 4; ++row) {
            float v = 0.0f;
            for (int k = 0; k < 4; ++k)
                v += a[k*4+row] * b[col*4+k];
            out[col*4+row] = v;
        }
}
}

namespace AnimatedLpg {

//==============================================
Scene::Scene() = default;

Scene::~Scene() = default;

void Scene::init(const WGPUDevice device, WGPUQueue queue)
{
    mDevice = device;
    mQueue  = queue;
}

//==============================================
//Setters
//==============================================
void Scene::setSurface(WGPUSurface surface) { mSurface = surface; }
void Scene::setShaderModule(const WGPUShaderModule shaderModule) { mShaderModule = shaderModule; }
void Scene::setPipelineDesc(const WGPURenderPipelineDescriptor& pipelineDesc) { mPipelineDesc = pipelineDesc; }

bool Scene::createShader()
{
#ifdef DEBUG
    const std::string dir = DEBUG_SHADER_DIR;
    mShaderPaths = {
        dir + "/common.wgsl",
        dir + "/lighting.wgsl",
        dir + "/mat_floor.wgsl",
        dir + "/vs_main.wgsl",
        dir + "/fs_main.wgsl"
    };
    mLastShaderWriteTime = latestWriteTime(mShaderPaths);
    mShaderModule        = ResourceManager::loadShaderModules(mShaderPaths, mDevice);
#else
    WGPUShaderModuleWGSLDescriptor shaderCodeDesc{};
    shaderCodeDesc.chain.next  = nullptr;
    shaderCodeDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
    shaderCodeDesc.code        = { shaderSource, strlen(shaderSource) };

    WGPUShaderModuleDescriptor shaderDesc{};
    shaderDesc.nextInChain     = &shaderCodeDesc.chain;
    mShaderModule              = wgpuDeviceCreateShaderModule(mDevice, &shaderDesc);
#endif

    if (!mShaderModule) {
        std::cerr << "Failed to create shader module." << std::endl;
        return false;
    }
    return true;
}


void Scene::terminate()
{
    if (mDepthTextureView)              { wgpuTextureViewRelease(mDepthTextureView); mDepthTextureView = nullptr; }
    if (mDepthTexture)                  { wgpuTextureDestroy(mDepthTexture); wgpuTextureRelease(mDepthTexture); mDepthTexture = nullptr; }
    if (mMsaaTextureView)               { wgpuTextureViewRelease(mMsaaTextureView); mMsaaTextureView = nullptr; }
    if (mMsaaTexture)                   { wgpuTextureDestroy(mMsaaTexture); wgpuTextureRelease(mMsaaTexture); mMsaaTexture = nullptr; }
    if (mSphereVertexBuffer)             { wgpuBufferRelease(mSphereVertexBuffer); mSphereVertexBuffer = nullptr; }
    if (mSphereIndexBuffer)              { wgpuBufferRelease(mSphereIndexBuffer); mSphereIndexBuffer = nullptr; }
    if (mFloorVertexBuffer)             { wgpuBufferRelease(mFloorVertexBuffer); mFloorVertexBuffer = nullptr; }
    if (mFloorIndexBuffer)              { wgpuBufferRelease(mFloorIndexBuffer); mFloorIndexBuffer = nullptr; }
    if (mBindGroup)                     { wgpuBindGroupRelease(mBindGroup); mBindGroup = nullptr; }
    if (mUniformBuffer)                 { wgpuBufferRelease(mUniformBuffer); mUniformBuffer = nullptr; }
    if (mPipeline)                      { wgpuRenderPipelineRelease(mPipeline); mPipeline = nullptr; }
    if (mSurface)                       { wgpuSurfaceUnconfigure(mSurface); wgpuSurfaceRelease(mSurface); mSurface = nullptr; }
}

void Scene::renderFrame(const float currentTime)
{
    #ifdef DEBUG
        auto writeTime = latestWriteTime(mShaderPaths);
        if (writeTime != mLastShaderWriteTime) {
            mLastShaderWriteTime = writeTime;
            reloadShader();
            return;
        }
    #endif

        if (!mPipeline) return;
        if (!mSurface) return;

        WGPUSurfaceTexture surfaceTexture = {};
        wgpuSurfaceGetCurrentTexture(mSurface, &surfaceTexture);

        if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) return;
        WGPUTextureViewDescriptor viewDesc = {};
        viewDesc.format = mSurfaceFormat;
        viewDesc.dimension = WGPUTextureViewDimension_2D;
        viewDesc.mipLevelCount = 1;
        viewDesc.arrayLayerCount = 1;
        viewDesc.aspect = WGPUTextureAspect_All;
        //=====================================
        //Push into buffer / fill buffer
        //=====================================
        setUniforms(mQueue, mUniformBuffer, currentTime);
        const WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDesc);
        wgpuTextureRelease(surfaceTexture.texture);

        WGPUCommandEncoderDescriptor encoderDesc = {};
        encoderDesc.label = WGPU_STR("Frame encoder");
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(mDevice, &encoderDesc);
        //======================================================
        //Color Attachment
        //======================================================
        WGPURenderPassColorAttachment colorAttachment = {};
        colorAttachment.view          = mMsaaTextureView; // draw into the multisampled buffer
        colorAttachment.resolveTarget = targetView;       // resolve down to the swapchain texture
        colorAttachment.loadOp        = WGPULoadOp_Clear;
        colorAttachment.storeOp       = WGPUStoreOp_Discard; // only the resolved result is kept
        colorAttachment.depthSlice    = WGPU_DEPTH_SLICE_UNDEFINED;

        WGPURenderPassDescriptor renderPassDesc = {};
        renderPassDesc.colorAttachmentCount     = 1;
        renderPassDesc.colorAttachments         = &colorAttachment;
        WGPURenderPassDepthStencilAttachment depthStencilAttachment = {};
        depthStencilAttachment.view             = mDepthTextureView;
        depthStencilAttachment.depthClearValue  = 1.0f;
        depthStencilAttachment.depthLoadOp      = WGPULoadOp_Clear;
        depthStencilAttachment.depthStoreOp     = WGPUStoreOp_Store;
        depthStencilAttachment.depthReadOnly    = false;
        depthStencilAttachment.stencilLoadOp    = WGPULoadOp_Undefined;
        depthStencilAttachment.stencilStoreOp   = WGPUStoreOp_Undefined;
        depthStencilAttachment.stencilReadOnly  = true;
        renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

        const WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

        wgpuRenderPassEncoderSetPipeline(renderPass, mPipeline);
        //Floor
        setItemBuffers(mFloorVertexBuffer, mFloorIndexBuffer, mFloorIndexCount, MAT_FLOOR, renderPass);
        setItemBuffers(mSphereVertexBuffer, mSphereIndexBuffer, mSphereIndexCount, MAT_FLOOR, renderPass);
        wgpuRenderPassEncoderEnd(renderPass);
        wgpuRenderPassEncoderRelease(renderPass);

        WGPUCommandBufferDescriptor cmdDesc = {};
        cmdDesc.label = WGPU_STR("Frame command buffer");
        const WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdDesc);
        wgpuCommandEncoderRelease(encoder);
        //==============================================
        //Process the frame
        //==============================================
        wgpuQueueSubmit(mQueue, 1, &command);
        wgpuCommandBufferRelease(command);
        wgpuTextureViewRelease(targetView);
        wgpuSurfacePresent(mSurface);
        wgpuDeviceTick(mDevice);
}

void Scene::setUniforms(WGPUQueue queue, const WGPUBuffer uniformBuffer, const float time)
{
    mUniforms.time        = time;
    std::memcpy(mUniforms.modelMatrix, kIdentity, sizeof(kIdentity));  // <-- add
    mUniforms.lightPos[0] = 0.0f;
    mUniforms.lightPos[1] = 0.35f;
    mUniforms.lightPos[2] = 0.0f;
    mUniforms.aspectRatio = static_cast<float>(mWidth) / static_cast<float>(mHeight);
    mUniforms.materialId  = MAT_FLOOR;
    updateViewMatrix();
    wgpuQueueWriteBuffer(queue, uniformBuffer,
                             MAT_FLOOR * mUniformStride, &mUniforms, sizeof(MyUniforms));
}

void Scene::ConfigureVertexLayout()
{
    // Attribute 0 — Position (location 0)
    mVertexAttribs[0].shaderLocation = 0;
    mVertexAttribs[0].format         = WGPUVertexFormat_Float32x3;
    mVertexAttribs[0].offset         = 0;
    // Attribute 1 — Normal (location 2)
    mVertexAttribs[1].shaderLocation = 2;
    mVertexAttribs[1].format         = WGPUVertexFormat_Float32x3;
    mVertexAttribs[1].offset         = 3 * sizeof(float);
    // Attribute 2 — Color (location 1)
    mVertexAttribs[2].shaderLocation = 1;
    mVertexAttribs[2].format         = WGPUVertexFormat_Float32x3;
    mVertexAttribs[2].offset         = 6 * sizeof(float);

    mVertexBufferLayouts.resize(1);
    mVertexBufferLayouts[0].attributeCount = 3;
    mVertexBufferLayouts[0].attributes     = mVertexAttribs.data();
    mVertexBufferLayouts[0].arrayStride    = 9 * sizeof(float);
    mVertexBufferLayouts[0].stepMode       = WGPUVertexStepMode_Vertex;

    mPipelineDesc.vertex.bufferCount = 1;
    mPipelineDesc.vertex.buffers     = mVertexBufferLayouts.data();
}

#ifdef DEBUG
void Scene::reloadShader()
{
    const WGPUShaderModule newModule = ResourceManager::loadShaderModules(mShaderPaths, mDevice);
    if (!newModule) {
        std::cerr << "Shader compile failed — keeping old pipeline." << std::endl;
        return;
    }
    wgpuShaderModuleRelease(mShaderModule);
    mShaderModule               = newModule;
    mPipelineDesc.vertex.module = mShaderModule;
    createPipeline();
    std::cout << "Shader reloaded." << std::endl;
}
#endif



void Scene::initializeScene()
{
    initializeFloor();
    initializeSphere();
}

bool Scene::createPipeline()
{
    if (mBindGroup)    { wgpuBindGroupRelease(mBindGroup);    mBindGroup     = nullptr; }
    if (mUniformBuffer){ wgpuBufferRelease(mUniformBuffer);   mUniformBuffer = nullptr; }
    if (mPipeline)     { wgpuRenderPipelineRelease(mPipeline); mPipeline     = nullptr; }

    static constexpr uint32_t kAlignment = 256;
    mUniformStride = (sizeof(MyUniforms) + kAlignment - 1) & ~(kAlignment - 1);

    WGPUBindGroupLayoutEntry bglEntry  = {};
    bglEntry.binding                   = 0;
    bglEntry.visibility                = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    bglEntry.buffer.type               = WGPUBufferBindingType_Uniform;
    bglEntry.buffer.hasDynamicOffset   = true;
    bglEntry.buffer.minBindingSize     = sizeof(MyUniforms);

    WGPUBindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount                    = 1;
    bglDesc.entries                       = &bglEntry;
    WGPUBindGroupLayout bgl = wgpuDeviceCreateBindGroupLayout(mDevice, &bglDesc);

    WGPUPipelineLayoutDescriptor layoutDesc = {};
    layoutDesc.bindGroupLayoutCount         = 1;
    layoutDesc.bindGroupLayouts             = &bgl;
    if (mPipelineDesc.layout) {
        wgpuPipelineLayoutRelease(mPipelineDesc.layout);
        mPipelineDesc.layout = nullptr;
    }
    mPipelineDesc.layout = wgpuDeviceCreatePipelineLayout(mDevice, &layoutDesc);

    mColorTarget.format        = mSurfaceFormat;
    mColorTarget.blend         = &mBlendState;
    mColorTarget.writeMask     = WGPUColorWriteMask_All;
    mPipelineDesc.vertex.module = mShaderModule;
    mFragmentState.module      = mShaderModule;
    mFragmentState.entryPoint  = WGPU_STR("fs_main");
    mFragmentState.targetCount = 1;
    mFragmentState.targets     = &mColorTarget;
    mFragmentState.constants   = nullptr;
    mPipelineDesc.fragment     = &mFragmentState;

    mPipeline = wgpuDeviceCreateRenderPipeline(mDevice, &mPipelineDesc);
    if (!mPipeline) {
        std::cerr << "Failed to create render pipeline." << std::endl;
        return false;
    }

    WGPUBufferDescriptor bufferDesc = {};
    bufferDesc.size                 = 11 * mUniformStride;  // one slot per material (0–10)
    bufferDesc.usage                = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
    mUniformBuffer                  = wgpuDeviceCreateBuffer(mDevice, &bufferDesc);

    WGPUBindGroupEntry entry = {};
    entry.binding            = 0;
    entry.buffer             = mUniformBuffer;
    entry.offset             = 0;
    entry.size               = sizeof(MyUniforms);

    WGPUBindGroupDescriptor bgDesc = {};
    bgDesc.layout                  = bgl;
    bgDesc.entryCount              = 1;
    bgDesc.entries                 = &entry;
    mBindGroup                     = wgpuDeviceCreateBindGroup(mDevice, &bgDesc);

    updateDepthTexture(mWidth, mHeight);

    wgpuBindGroupLayoutRelease(bgl);
    return true;
}

void Scene::updateDepthTexture(const uint32_t width, const uint32_t height)
{
    mWidth = width;
    mHeight = height;
    if (mDepthTextureView) { wgpuTextureViewRelease(mDepthTextureView); mDepthTextureView = nullptr; }
    if (mDepthTexture)     { wgpuTextureDestroy(mDepthTexture); wgpuTextureRelease(mDepthTexture); mDepthTexture = nullptr; }
    if (mMsaaTextureView)  { wgpuTextureViewRelease(mMsaaTextureView); mMsaaTextureView = nullptr; }
    if (mMsaaTexture)      { wgpuTextureDestroy(mMsaaTexture); wgpuTextureRelease(mMsaaTexture); mMsaaTexture = nullptr; }

    WGPUTextureDescriptor depthDesc{};
    depthDesc.usage         = WGPUTextureUsage_RenderAttachment;
    depthDesc.dimension     = WGPUTextureDimension_2D;
    depthDesc.size          = { mWidth, mHeight, 1 };
    depthDesc.format        = WGPUTextureFormat_Depth24Plus;
    depthDesc.mipLevelCount = 1;
    depthDesc.sampleCount   = kMSAASamples;   // must match the color target's sample count
    mDepthTexture           = wgpuDeviceCreateTexture(mDevice, &depthDesc);

    WGPUTextureViewDescriptor dvDesc{};
    dvDesc.format          = WGPUTextureFormat_Depth24Plus;
    dvDesc.dimension       = WGPUTextureViewDimension_2D;
    dvDesc.mipLevelCount   = 1;
    dvDesc.arrayLayerCount = 1;
    dvDesc.aspect          = WGPUTextureAspect_DepthOnly;
    mDepthTextureView      = wgpuTextureCreateView(mDepthTexture, &dvDesc);

    // Multisampled color buffer matching the surface format. Drawing targets this,
    // and the render pass resolves it down to the single-sampled swapchain texture.
    WGPUTextureDescriptor msaaDesc{};
    msaaDesc.usage         = WGPUTextureUsage_RenderAttachment;
    msaaDesc.dimension     = WGPUTextureDimension_2D;
    msaaDesc.size          = { mWidth, mHeight, 1 };
    msaaDesc.format        = mSurfaceFormat;
    msaaDesc.mipLevelCount = 1;
    msaaDesc.sampleCount   = kMSAASamples;
    mMsaaTexture           = wgpuDeviceCreateTexture(mDevice, &msaaDesc);

    WGPUTextureViewDescriptor mvDesc{};
    mvDesc.format          = mSurfaceFormat;
    mvDesc.dimension       = WGPUTextureViewDimension_2D;
    mvDesc.mipLevelCount   = 1;
    mvDesc.arrayLayerCount = 1;
    mvDesc.aspect          = WGPUTextureAspect_All;
    mMsaaTextureView       = wgpuTextureCreateView(mMsaaTexture, &mvDesc);
}

void Scene::initializeFloor()
{
    std::cout << "Initializing floor" << std::endl;
    std::vector<FloorVertex> vertices;
    std::vector<FloorIndex>  indices;

    circularFloor::buildFloor(vertices, indices, 0.95f, 64);

    mFloorIndexCount = static_cast<uint32_t>(indices.size());

    WGPUBufferDescriptor bd{};
    bd.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
    bd.size  = vertices.size() * sizeof(FloorVertex);
    mFloorVertexBuffer = wgpuDeviceCreateBuffer(mDevice, &bd);
    wgpuQueueWriteBuffer(mQueue, mFloorVertexBuffer, 0, vertices.data(), bd.size);

    bd.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
    bd.size  = (indices.size() * sizeof(FloorIndex) + 3) & ~3ULL;
    mFloorIndexBuffer = wgpuDeviceCreateBuffer(mDevice, &bd);
    wgpuQueueWriteBuffer(mQueue, mFloorIndexBuffer, 0, indices.data(), bd.size);
}

void Scene::initializeSphere()
{
    std::cout << "Initializing Sphere" << std::endl;
    std::vector<SphereVertex> vertices;
    std::vector<SphereIndex>  indices;

    SphereGeometry::buildSphere(vertices, indices);

    mSphereIndexCount = static_cast<uint32_t>(indices.size());

    WGPUBufferDescriptor bd{};
    bd.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
    bd.size  = vertices.size() * sizeof(SphereVertex);
    mSphereVertexBuffer = wgpuDeviceCreateBuffer(mDevice, &bd);
    wgpuQueueWriteBuffer(mQueue, mSphereVertexBuffer, 0, vertices.data(), bd.size);

    bd.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
    bd.size  = (indices.size() * sizeof(FloorIndex) + 3) & ~3ULL;
    mSphereIndexBuffer = wgpuDeviceCreateBuffer(mDevice, &bd);
    wgpuQueueWriteBuffer(mQueue, mSphereIndexBuffer, 0, indices.data(), bd.size);
}

void Scene::updateViewMatrix()
{
    const float yaw          = mCameraState.angleX;
    const float cameraX      = mCameraState.posX;
    constexpr float cameraY  = CameraState::eyeY;
    const float cameraZ      = mCameraState.posZ;

    const float tx = cameraX + sinf(yaw);
    const float tz = cameraZ - cosf(yaw);

    float view[16], proj[16];
    buildLookAt(view, cameraX, cameraY, cameraZ, tx, cameraY, tz);
    buildPerspective(proj, 1.047f, mUniforms.aspectRatio, 0.1f, 8.0f);
    mulMat4(mUniforms.viewProjMatrix, proj, view);
    std::memcpy(mUniforms.projMatrix, proj, sizeof(proj));
}

void Scene::onMouseMove(const float xpos, const float /*ypos*/)
{
    if (mDrag.active)
    {
        mCameraState.angleX = mDrag.startAngleX + (xpos - mDrag.startMouseX) * mDrag.sensitivity;
        updateViewMatrix();
    }
}

void Scene::onMouseButton(const int button, const bool isPressed, const float xpos, float /*ypos*/)
{
    if (button == 0)
    {
        if (isPressed)
        {
            mDrag.active      = true;
            mDrag.startMouseX = xpos;
            mDrag.startAngleX = mCameraState.angleX;
        }
        else
        {
            mDrag.active = false;
        }
    }
}

void Scene::onScroll(const float deltaX, const float deltaY)
{
    const float yaw   = mCameraState.angleX;
    const float speed = mDrag.scrollSensitivity;

    mCameraState.posX   +=  sinf(yaw) * deltaY * speed;
    mCameraState.posZ   += -cosf(yaw) * deltaY * speed;
    mCameraState.angleX +=  deltaX * mDrag.turnSensitivity * speed;

    updateViewMatrix();
}

}