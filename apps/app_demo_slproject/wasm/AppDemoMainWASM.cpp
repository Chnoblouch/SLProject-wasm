#include <SLInterface.h>
#include <SLScene.h>
#include <SLLightSpot.h>
#include <SLBox.h>
#include <SLRectangle.h>
#include <SLAssetStore.h>
#include <AppDemo.h>
#include <AppDemoSceneView.h>
#include <AppDemoGui.h>

#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>

#include <iostream>
#include <fstream>
#include <atomic>

//-----------------------------------------------------------------------------
// Global application variables
static GLFWwindow* window;                     //!< The global glfw window handle
static SLint       svIndex;                    //!< SceneView index
static SLint       scrWidth;                   //!< Window width at start up
static SLint       scrHeight;                  //!< Window height at start up
static SLbool      fixAspectRatio;             //!< Flag if wnd aspect ratio should be fixed
static SLfloat     scrWdivH;                   //!< aspect ratio screen width divided by height
static SLint       dpi = 142;                  //!< Dot per inch resolution of screen
static SLint       startX;                     //!< start position x in pixels
static SLint       startY;                     //!< start position y in pixels
static SLint       mouseX;                     //!< Last mouse position x in pixels
static SLint       mouseY;                     //!< Last mouse position y in pixels
static SLVec2i     touch2;                     //!< Last finger touch 2 position in pixels
static SLVec2i     touchDelta;                 //!< Delta between two fingers in x
static SLint       lastWidth;                  //!< Last window width in pixels
static SLint       lastHeight;                 //!< Last window height in pixels
static SLfloat     lastMouseDownTime = 0.0f;   //!< Last mouse press time
static SLKey       modifiers         = K_none; //!< last modifier keys
static SLbool      fullscreen        = false;  //!< flag if window is in fullscreen mode

extern void appDemoLoadScene(SLAssetManager* am,
                             SLScene*        s,
                             SLSceneView*    sv,
                             SLSceneID       sceneID);

static void onResize(GLFWwindow* myWindow, int width, int height)
{
    if (AppDemo::sceneViews.empty()) return;
    SLSceneView* sv = AppDemo::sceneViews[svIndex];

    if (fixAspectRatio)
    {
        // correct target width and height
        if ((float)height * scrWdivH <= (float)width)
        {
            width  = (int)((float)height * scrWdivH);
            height = (int)((float)width / scrWdivH);
        }
        else
        {
            height = (int)((float)width / scrWdivH);
            width  = (int)((float)height * scrWdivH);
        }
    }

    lastWidth  = width;
    lastHeight = height;

    // width & height are in screen coords.
    slResize(svIndex, width, height);
}

static void onMouseButton(GLFWwindow* myWindow,
                          int         button,
                          int         action,
                          int         mods)
{
    SLint x = mouseX;
    SLint y = mouseY;
    startX  = x;
    startY  = y;

    // Translate modifiers
    modifiers = K_none;
    if ((uint)mods & (uint)GLFW_MOD_SHIFT) modifiers = (SLKey)(modifiers | K_shift);
    if ((uint)mods & (uint)GLFW_MOD_CONTROL) modifiers = (SLKey)(modifiers | K_ctrl);
    if ((uint)mods & (uint)GLFW_MOD_ALT) modifiers = (SLKey)(modifiers | K_alt);

    if (action == GLFW_PRESS)
    {
        SLfloat mouseDeltaTime = (SLfloat)glfwGetTime() - lastMouseDownTime;
        lastMouseDownTime      = (SLfloat)glfwGetTime();

        // handle double click
        if (mouseDeltaTime < 0.3f)
        {
            switch (button)
            {
                case GLFW_MOUSE_BUTTON_LEFT:
                    slDoubleClick(svIndex, MB_left, x, y, modifiers);
                    break;
                case GLFW_MOUSE_BUTTON_RIGHT:
                    slDoubleClick(svIndex, MB_right, x, y, modifiers);
                    break;
                case GLFW_MOUSE_BUTTON_MIDDLE:
                    slDoubleClick(svIndex, MB_middle, x, y, modifiers);
                    break;
                default: break;
            }
        }
        else // normal mouse clicks
        {
            switch (button)
            {
                case GLFW_MOUSE_BUTTON_LEFT:
                    if (modifiers & K_alt && modifiers & K_ctrl)
                        slTouch2Down(svIndex, x - 20, y, x + 20, y);
                    else
                        slMouseDown(svIndex, MB_left, x, y, modifiers);
                    break;
                case GLFW_MOUSE_BUTTON_RIGHT:
                    slMouseDown(svIndex, MB_right, x, y, modifiers);
                    break;
                case GLFW_MOUSE_BUTTON_MIDDLE:
                    slMouseDown(svIndex, MB_middle, x, y, modifiers);
                    break;
                default: break;
            }
        }
    }
    else
    { // flag end of mouse click for long touches
        startX = -1;
        startY = -1;

        switch (button)
        {
            case GLFW_MOUSE_BUTTON_LEFT:
                slMouseUp(svIndex, MB_left, x, y, modifiers);
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                slMouseUp(svIndex, MB_right, x, y, modifiers);
                break;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                slMouseUp(svIndex, MB_middle, x, y, modifiers);
                break;
            default: break;
        }
    }
}

static void onMouseMove(GLFWwindow* myWindow,
                        double      x,
                        double      y)
{
    // x & y are in screen coords.
    mouseX = (int)x;
    mouseY = (int)y;

    if (modifiers & K_alt && modifiers & K_ctrl)
        slTouch2Move(svIndex, (int)(x - 20), (int)y, (int)(x + 20), (int)y);
    else
        slMouseMove(svIndex, (int)x, (int)y);
}

static void onMouseWheel(GLFWwindow* myWindow,
                         double      xscroll,
                         double      yscroll)
{
    // make sure the delta is at least one integer
    int dY = (int)yscroll;
    if (dY == 0) dY = (int)(Utils::sign(yscroll));

    slMouseWheel(svIndex, -dY, modifiers);
}

void onGLFWError(int error, const char* description)
{
    fputs(description, stderr);
}

void appDemoLoadSceneEmscripten(SLAssetManager* am,
                                SLScene*        s,
                                SLSceneView*    sv,
                                SLSceneID       sceneID)
{
    std::cout << "loading scene..." << std::endl;

    s->init(am);

    // Set scene name and info string
    s->name("Minimal Scene Test");
    s->info("Minimal scene with a texture mapped rectangle with a point light source.\n"
            "You can find all other test scenes in the menu File > Load Test Scenes."
            "You can jump to the next scene with the Shift-Alt-CursorRight.\n"
            "You can open various info windows under the menu Infos. You can drag, dock and stack them on all sides.\n"
            "You can rotate the scene with click and drag on the left mouse button (LMB).\n"
            "You can zoom in/out with the mousewheel. You can pan with click and drag on the middle mouse button (MMB).\n");

    // Create a scene group node
    SLNode* scene = new SLNode("scene node");
    s->root3D(scene);

    // Create textures and materials
    SLGLTexture* texC = new SLGLTexture(am, AppDemo::texturePath + "earth2048_C.png");
    SLMaterial*  m1   = new SLMaterial(am, "m1", texC);

    // Create a light source node
    SLLightSpot* light1 = new SLLightSpot(am, s, 0.3f);
    light1->translation(0, 0, 5);
    light1->name("light node");
    scene->addChild(light1);

    // Create meshes and nodes
    SLMesh* rectMesh = new SLRectangle(am,
                                        SLVec2f(-5, -5),
                                        SLVec2f(5, 5),
                                        25,
                                        25,
                                        "rectangle mesh",
                                        m1);
    SLNode* rectNode = new SLNode(rectMesh, "rectangle node");
    scene->addChild(rectNode);

    // Set background color and the root scene node
    sv->sceneViewCamera()->background().colors(SLCol4f(0.7f, 0.7f, 0.7f),
                                                SLCol4f(0.2f, 0.2f, 0.2f));
    // Save energy
    sv->doWaitOnIdle(true);
}

SLSceneView* createAppDemoSceneView(SLScene* scene, int curDPI, SLInputManager& inputManager)
{
    return new AppDemoSceneView(scene, curDPI, inputManager);
}

EM_JS(int, getViewportWidth, (), {
    return window.innerWidth;
});

EM_JS(int, getViewportHeight, (), {
    return window.innerHeight;
});

SLbool onPaint() {
    int width;
    int height;
    glfwGetWindowSize(window, &width, &height);

    int newWidth = getViewportWidth();
    int newHeight = getViewportHeight();
    if(newWidth != width || newHeight != height) glfwSetWindowSize(window, newWidth, newHeight);

    slPaintAllViews();
    glfwSwapBuffers(window);
    return true;
}

EM_BOOL onLoop(double time, void* userData) {
    onPaint();
    return EM_TRUE;
}

void runApp()
{
    glfwInit();
    
    int windowWidth = 1080;
    int windowHeight = 720;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow(windowWidth, windowHeight, "SLProject", NULL, NULL);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSwapInterval(1);

    SLVstring args;

    slCreateAppAndScene(
        args,
        "data/",
        "data/shaders/",
        "data/models/",
        "data/images/textures/",
        "data/images/fonts/",
        "data/videos/",
        "config/",
        "AppDemoEmscripten",
        (void*)appDemoLoadScene
    );

    slCreateSceneView(
        AppDemo::assetManager,
        AppDemo::scene,
        windowWidth,
        windowHeight,
        1,
        (SLSceneID)SID_Minimal,
        (void*)&onPaint,
        nullptr,
        (void*)createAppDemoSceneView,
        (void*)AppDemoGui::build,
        (void*)AppDemoGui::loadConfig,
        (void*)AppDemoGui::saveConfig
    );

    glfwSetWindowSizeCallback(window, onResize);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window, onMouseMove);
    glfwSetScrollCallback(window, onMouseWheel);

    glEnable(GL_DEPTH_TEST);

    // We cannot loop ourselves because that would block the page,
    // but we can register an update function to be called in every iteration
    // of the JavaScript event loop.
    emscripten_request_animation_frame_loop(onLoop, nullptr);

    //glfwTerminate();
}

int main(void)
{
    runApp();
    return 0;
}