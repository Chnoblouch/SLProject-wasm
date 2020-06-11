#define oldProject 0

#define GLFW_INCLUDE_VULKAN
// #include <GL/glew.h> // OpenGL headers
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <string>
#include <CVImage.h> // Image class for image loading
#include <math/SLVec3.h>
#include <glUtils.h>
#include <Utils.h>

#include "Instance.h"
#include "Device.h"
#include "Swapchain.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "ShaderModule.h"
#include "Pipeline.h"
#include "Framebuffer.h"
#include "TextureImage.h"
#include "Sampler.h"
#include "IndexBuffer.h"
#include "UniformBuffer.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "VertexBuffer.h"

//-----------------------------------------------------------------------------
//////////////////////
// Global Variables //
//////////////////////

struct Vertex;

const int WINDOW_WIDTH   = 800;
const int WINDOW_HEIGHT  = 600;
string    vertShaderPath = SLstring(SL_PROJECT_ROOT) + "/data/shaders/vertShader.vert.spv";
string    fragShaderPath = SLstring(SL_PROJECT_ROOT) + "/data/shaders/fragShader.frag.spv";

GLFWwindow* window;
#if oldProject
vkUtils renderer;
#endif
// Plane
// const vector<Vertex> vertices = {{{-1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
//                                  {{1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
//                                  {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
//                                  {{-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}};
// const vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

// Camera
SLMat4f _viewMatrix;
float   _camZ = 6.0f;

// Mouse
int  _startX, _startY;
int  _mouseX, _mouseY;
int  _deltaX, _deltaY;
int  _rotX, _rotY;
bool _mouseLeftDown;

//-----------------------------------------------------------------------------
void buildSphere(float radius, GLuint stacks, GLuint slices, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
{
    assert(stacks > 3 && slices > 3);

    // create vertex array
    unsigned int numV = (stacks + 1) * (slices + 1);
    // std::vector<Vertex> vertices(numV);
    vertices.resize(numV);

    float  theta, dtheta; // angles around x-axis
    float  phi, dphi;     // angles around z-axis
    GLuint i, j;          // loop counters
    GLuint iv = 0;

    // init start values
    theta  = 0.0f;
    dtheta = Utils::PI / stacks;
    dphi   = 2.0f * Utils::PI / slices;

    // Define vertex position & normals by looping through all stacks
    for (i = 0; i <= stacks; ++i)
    {
        float sin_theta = sin(theta);
        float cos_theta = cos(theta);
        phi             = 0.0f;

        // Loop through all slices
        for (j = 0; j <= slices; ++j)
        {
            if (j == slices) phi = 0.0f;

            // define first the normal with length 1
            vertices[iv].norm.x = sin_theta * cos(phi);
            vertices[iv].norm.y = sin_theta * sin(phi);
            vertices[iv].norm.z = cos_theta;

            // set the vertex position w. the scaled normal
            vertices[iv].pos.x = radius * vertices[iv].norm.x;
            vertices[iv].pos.y = radius * vertices[iv].norm.y;
            vertices[iv].pos.z = radius * vertices[iv].norm.z;

            // set the texture coords.
            vertices[iv].texCoord.x = asin(vertices[iv].norm.x) / Utils::PI + 0.5f;
            vertices[iv].texCoord.y = -asin(vertices[iv].norm.y) / Utils::PI + 0.5f;

            phi += dphi;
            iv++;
        }
        theta += dtheta;
    }

    // create Index array x
    unsigned int numI = (GLuint)(slices * stacks * 2 * 3);
    indices.resize(numI);
    GLuint ii = 0, iV1, iV2;

    for (i = 0; i < stacks; ++i)
    {
        // index of 1st & 2nd vertex of stack
        iV1 = i * (slices + 1);
        iV2 = iV1 + slices + 1;

        for (j = 0; j < slices; ++j)
        { // 1st triangle ccw
            indices[ii++] = iV1 + j;
            indices[ii++] = iV2 + j;
            indices[ii++] = iV2 + j + 1;
            // 2nd triangle ccw
            indices[ii++] = iV1 + j;
            indices[ii++] = iV2 + j + 1;
            indices[ii++] = iV1 + j + 1;
        }
    }
}

//-----------------------------------------------------------------------------
void onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
    SLint x = _mouseX;
    SLint y = _mouseY;

    _mouseLeftDown = (action == GLFW_PRESS);
    if (_mouseLeftDown)
    {
        _startX = x;
        _startY = y;
    }
    else
    {
        _rotX += _deltaX;
        _rotY += _deltaY;
        _deltaX = 0;
        _deltaY = 0;
    }
}
//-----------------------------------------------------------------------------
void onMouseMove(GLFWwindow* window, double x, double y)
{
    _mouseX = (int)x;
    _mouseY = (int)y;

    if (_mouseLeftDown)
    {
        _deltaY = (int)x - _startX;
        _deltaX = (int)y - _startY;
    }
}
//-----------------------------------------------------------------------------
void onMouseWheel(GLFWwindow* window, double xScroll, double yScroll)
{
    _camZ -= (SLfloat)Utils::sign(yScroll) * 0.1f;
}
//-----------------------------------------------------------------------------
float calcFPS(float deltaTime)
{
    const SLint    FILTERSIZE = 60;
    static SLfloat frameTimes[FILTERSIZE];
    static SLuint  frameNo = 0;

    frameTimes[frameNo % FILTERSIZE] = deltaTime;
    float sumTime                    = 0.0f;

    for (SLuint i = 0; i < FILTERSIZE; ++i)
        sumTime += frameTimes[i];

    frameNo++;
    float frameTimeSec = sumTime / (SLfloat)FILTERSIZE;
    float fps          = 1 / frameTimeSec;

    return fps;
}
//-----------------------------------------------------------------------------
void printFPS()
{
    char         title[255];
    static float lastTimeSec = 0.0f;
    float        timeNowSec  = (float)glfwGetTime();
    float        fps         = calcFPS(timeNowSec - lastTimeSec);
    sprintf(title, "fps: %5.0f", fps);
    glfwSetWindowTitle(window, title);
    lastTimeSec = timeNowSec;
}
//-----------------------------------------------------------------------------
void initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WINDOW_WIDTH,
                              WINDOW_HEIGHT,
                              "Vulkan",
                              nullptr,
                              nullptr);

    glfwSetWindowSizeLimits(window, 100, 100, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window, onMouseMove);
    glfwSetScrollCallback(window, onMouseWheel);
}
//-----------------------------------------------------------------------------
void updateCamera()
{
    _viewMatrix.identity();
    _viewMatrix.translate(0.0f, 0.0f, -_camZ);
    _viewMatrix.rotate((float)(_rotX + _deltaX), 1.0f, 0.0f, 0.0f);
    _viewMatrix.rotate((float)(_rotY + _deltaY), 0.0f, 1.0f, 0.0f);
}
//-----------------------------------------------------------------------------
int main()
{
    initWindow();
    // Needed data
    std::vector<Vertex>   vertices;
    std::vector<uint16_t> indices;
    buildSphere(1.0f, 32, 32, vertices, indices);

    const vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const vector<const char*> deviceExtensions = {"VK_KHR_swapchain", "VK_KHR_maintenance1"};
    // Setting up vulkan
    Instance     instance = Instance("Test", deviceExtensions, validationLayers);
    VkSurfaceKHR surface;
    glfwCreateWindowSurface(instance.handle, window, nullptr, &surface);
    Device     device     = Device(instance, instance.physicalDevice, surface, deviceExtensions);
    Swapchain  swapchain  = Swapchain(device, window);
    RenderPass renderPass = RenderPass(device, swapchain);

    // Shader program setup
    DescriptorSetLayout descriptorSetLayout = DescriptorSetLayout(device);
    ShaderModule        vertShaderModule    = ShaderModule(device, vertShaderPath);
    ShaderModule        fragShaderModule    = ShaderModule(device, fragShaderPath);
    Pipeline            pipeline            = Pipeline(device, swapchain, descriptorSetLayout, renderPass, vertShaderModule, fragShaderModule);
    Framebuffer         framebuffer         = Framebuffer(device, renderPass, swapchain);

    // Texture setup
    CVImage texture;
    texture.load(SLstring(SL_PROJECT_ROOT) + "/data/images/textures/tree1_1024_C.png", false, false);
    TextureImage textureImage   = TextureImage(device, texture.data(), texture.width(), texture.height());
    Sampler      textureSampler = Sampler(device);

    // Mesh setup
    Buffer indexBuffer = Buffer(device);
    indexBuffer.createIndexBuffer(indices);
    UniformBuffer  uniformBuffer  = UniformBuffer(device, swapchain, _viewMatrix);
    DescriptorPool descriptorPool = DescriptorPool(device, swapchain);
    DescriptorSet  descriptorSet  = DescriptorSet(device, swapchain, descriptorSetLayout, descriptorPool, uniformBuffer, textureSampler, textureImage);
    Buffer         vertexBuffer   = Buffer(device);
    vertexBuffer.createVertexBuffer(vertices);
    // Draw call setup
    CommandBuffer commandBuffer = CommandBuffer(device);
    commandBuffer.setVertices(swapchain, framebuffer, renderPass, vertexBuffer, indexBuffer, pipeline, descriptorSet, (int)indices.size());
    device.createSyncObjects(swapchain);

    // Render
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        updateCamera();
        pipeline.draw(swapchain, uniformBuffer, commandBuffer);
        printFPS();
    }
    // Destroy
    device.waitIdle();

    framebuffer.destroy();
    commandBuffer.destroy();
    pipeline.destroy();
    renderPass.destroy();
    swapchain.destroy();
    uniformBuffer.destroy();
    descriptorPool.destroy();
    descriptorSetLayout.destroy();
    textureImage.destroy();
    textureSampler.destroy();
    indexBuffer.destroy();
    vertexBuffer.destroy();
    vertShaderModule.destroy();
    fragShaderModule.destroy();
    device.destroy();
    instance.destroy();

    return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------
