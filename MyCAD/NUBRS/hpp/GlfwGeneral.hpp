#pragma once
#include "VKBase.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib")

using namespace vulkan;

//窗口指针
GLFWwindow* pWindow;
//显示器指针
GLFWmonitor* pMonitor;
//窗口标题
const char* windowTitle = "MyCAD";

bool InitializeWindow(VkExtent2D size, bool fullScreen = false, bool isResizable = true, bool limitFrameRate = true) {
	if (!glfwInit())
	{
		std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to initialize GLFW!\n");
		return false;
	}

	//告诉glfw不需要opengl上下文
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//设置窗口拉伸
	glfwWindowHint(GLFW_RESIZABLE, isResizable);
	
	pMonitor = glfwGetPrimaryMonitor();
	//拿到视频模式，设置分辨率有用
	const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);

	//第四个参数是全屏模式的显示器，第五个是其他窗口的指针
	pWindow = fullScreen ? glfwCreateWindow(pMode->width, pMode->height, windowTitle, pMonitor, nullptr) :
		glfwCreateWindow(size.width, size.height, windowTitle, nullptr, nullptr);
	if (!pWindow) {
		std::cout << std::format("[ InitializeWindow ]\nFailed to create a glfw window!\n");
		glfwTerminate();
		return false;
	}
	

	uint32_t extensionCount = 0;
	const char** extensionNames;
	extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);
	if (!extensionNames) {
		outStream << std::format("[ InitializeWindow ]\nVulkan is not available on this machine!\n");
		glfwTerminate();
		return false;
	}
	for (size_t i = 0; i < extensionCount; i++)
		graphicsBase::Base().AddInstanceExtension(extensionNames[i]);
	graphicsBase::Base().AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	//在创建window surface前创建Vulkan实例
	graphicsBase::Base().UseLatestApiVersion();
	if (graphicsBase::Base().CreateInstance())
		return false;

	//创建window surface
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	if (VkResult result = glfwCreateWindowSurface(graphicsBase::Base().Instance(), pWindow, nullptr, &surface)) {
		std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to create a window surface!\nError code: {}\n", int32_t(result));
		glfwTerminate();
		return false;
	}
	graphicsBase::Base().Surface(surface);

	//通过用||操作符短路执行来省去几行
	if (//获取物理设备，并使用列表中的第一个物理设备，这里不考虑以下任意函数失败后更换物理设备的情况
		graphicsBase::Base().GetPhysicalDevices() ||
		//一个true一个false，暂时不需要计算用的队列
		graphicsBase::Base().DeterminePhysicalDevice(0, true, false) ||
		//创建逻辑设备
		graphicsBase::Base().CreateDevice())
		return false;
	//----------------------------------------

	if (graphicsBase::Base().CreateSwapchain(limitFrameRate))
		return false;

	return true;
}
bool TerminateWindow() {
	graphicsBase::Base().WaitIdle();
	glfwTerminate();
	return true;
}

//给窗口显示帧率
void TitleFps() {
	//因为是静态变量，所以只会在初始化时赋值
	static double time0 = glfwGetTime();
	static double time1;
	static double dt;
	static int dframe = -1;
	static std::stringstream info;
	time1 = glfwGetTime();
	dframe++;
	if ((dt = time1 - time0) >= 1) {
		info.precision(1);
		info << windowTitle << "    " << std::fixed << dframe / dt << "FPS";
		glfwSetWindowTitle(pWindow, info.str().c_str());
		info.str("");
		time0 = time1;
		dframe = 0;
	}
}





