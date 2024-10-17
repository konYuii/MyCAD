#pragma once
#include "VKBase.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib")

using namespace vulkan;

//����ָ��
GLFWwindow* pWindow;
//��ʾ��ָ��
GLFWmonitor* pMonitor;
//���ڱ���
const char* windowTitle = "MyCAD";

bool InitializeWindow(VkExtent2D size, bool fullScreen = false, bool isResizable = true, bool limitFrameRate = true) {
	if (!glfwInit())
	{
		std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to initialize GLFW!\n");
		return false;
	}

	//����glfw����Ҫopengl������
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//���ô�������
	glfwWindowHint(GLFW_RESIZABLE, isResizable);
	
	pMonitor = glfwGetPrimaryMonitor();
	//�õ���Ƶģʽ�����÷ֱ�������
	const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);

	//���ĸ�������ȫ��ģʽ����ʾ������������������ڵ�ָ��
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
	//�ڴ���window surfaceǰ����Vulkanʵ��
	graphicsBase::Base().UseLatestApiVersion();
	if (graphicsBase::Base().CreateInstance())
		return false;

	//����window surface
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	if (VkResult result = glfwCreateWindowSurface(graphicsBase::Base().Instance(), pWindow, nullptr, &surface)) {
		std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to create a window surface!\nError code: {}\n", int32_t(result));
		glfwTerminate();
		return false;
	}
	graphicsBase::Base().Surface(surface);

	//ͨ����||��������·ִ����ʡȥ����
	if (//��ȡ�����豸����ʹ���б��еĵ�һ�������豸�����ﲻ�����������⺯��ʧ�ܺ���������豸�����
		graphicsBase::Base().GetPhysicalDevices() ||
		//һ��trueһ��false����ʱ����Ҫ�����õĶ���
		graphicsBase::Base().DeterminePhysicalDevice(0, true, false) ||
		//�����߼��豸
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

//��������ʾ֡��
void TitleFps() {
	//��Ϊ�Ǿ�̬����������ֻ���ڳ�ʼ��ʱ��ֵ
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





