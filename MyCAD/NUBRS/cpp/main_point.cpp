#include "../hpp/GlfwGeneral.hpp"
#include "../hpp/EasyVulkan.h"
#include "../hpp/NUBRS.h"
using namespace vulkan;
using namespace NUBRS;

#define SAMPLE 1000

pipelineLayout pipelineLayout_point;
pipeline pipeline_point;
const auto& RenderPassAndFramebuffers() {
	static const auto& rpwf = easyVulkan::CreateRpwf_Screen();
	return rpwf;
}
void CreateLayout() {
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayout_point.Create(pipelineLayoutCreateInfo);
}
void CreatePipeline() {
	static shaderModule vert("./shader/NUBRS.vert.spv");
	static shaderModule frag("./shader/NUBRS.frag.spv");
	static VkPipelineShaderStageCreateInfo shaderStageCreateInfos[2] = {
		vert.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
		frag.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)
	};
	auto Create = [] {
		graphicsPipelineCreateInfoPack pipelineCiPack;
		pipelineCiPack.createInfo.layout = pipelineLayout_point;
		pipelineCiPack.createInfo.renderPass = RenderPassAndFramebuffers().renderPass;
		//添加顶点描述
		pipelineCiPack.vertexInputBindings.emplace_back(0, sizeof(Point_2D), VK_VERTEX_INPUT_RATE_VERTEX);
		pipelineCiPack.vertexInputAttributes.emplace_back(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Point_2D, position));
		pipelineCiPack.vertexInputAttributes.emplace_back(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Point_2D, tangent));
		pipelineCiPack.vertexInputAttributes.emplace_back(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Point_2D, normal));

		pipelineCiPack.inputAssemblyStateCi.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		pipelineCiPack.viewports.emplace_back(0.f, 0.f, float(windowSize.width), float(windowSize.height), 0.f, 1.f);
		pipelineCiPack.scissors.emplace_back(VkOffset2D{}, windowSize);
		pipelineCiPack.multisampleStateCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		pipelineCiPack.colorBlendAttachmentStates.push_back({ .colorWriteMask = 0b1111 });
		pipelineCiPack.UpdateAllArrays();
		pipelineCiPack.createInfo.stageCount = 2;
		pipelineCiPack.createInfo.pStages = shaderStageCreateInfos;
		pipeline_point.Create(pipelineCiPack);
		};
	auto Destroy = [] {
		pipeline_point.~pipeline();
		};
	graphicsBase::Base().AddCallback_CreateSwapchain(Create);
	graphicsBase::Base().AddCallback_DestroySwapchain(Destroy);
	Create();
}

int main() {
	if (!InitializeWindow({ 1280, 720 }))
		return -1;

	const auto& [renderPass, framebuffers] = RenderPassAndFramebuffers();
	CreateLayout();
	CreatePipeline();

	fence fence;
	semaphore semaphore_imageIsAvailable;
	semaphore semaphore_renderingIsOver;

	commandBuffer commandBuffer;
	commandPool commandPool(graphicsBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	commandPool.AllocateBuffers(commandBuffer);

	VkClearValue clearColor = { .color = { 0.f, 0.f, 0.f, 1.f } };

	//画幂基曲线
	//std::vector<vertex_2D> control_points = {
	//	{{-0.5f, 0.5f}},
	//	{{-0.2f, -0.5f}},
	//	{{0.2f, -0.5f}},
	//	{{0.5f, 0.5f}},
	//};
	//PowerCurve_2D curve(control_points);


	//画Bezier曲线
	std::vector<vertex_2D> control_points = {
		{{-0.5f, 0.5f}},
		{{-0.2f, -0.5f}},
		{{0.2f, -0.5f}},
		{{0.5f, 0.5f}},
	};
	BezierCurve_2D curve(control_points);


	//要把vector转成数组，不知道为啥vector传不进vulkan
	std::vector<Point_2D> points(SAMPLE);
	curve.ComputeAllPoints(points);
	Point_2D pointsArray[SAMPLE];
	memcpy(pointsArray, &points[0], points.size() * sizeof(Point_2D));
	vertexBuffer vertexBuffer(sizeof pointsArray);
	vertexBuffer.TransferData(pointsArray);



	while (!glfwWindowShouldClose(pWindow)) {
		while (glfwGetWindowAttrib(pWindow, GLFW_ICONIFIED))
			glfwWaitEvents();

		graphicsBase::Base().SwapImage(semaphore_imageIsAvailable);
		auto i = graphicsBase::Base().CurrentImageIndex();

		commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		renderPass.CmdBegin(commandBuffer, framebuffers[i], { {}, windowSize }, clearColor);
		//绑定顶点
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffer.Address(), &offset);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_point);
		vkCmdDraw(commandBuffer, points.size(), 1, 0, 0);
		renderPass.CmdEnd(commandBuffer);
		commandBuffer.End();

		graphicsBase::Base().SubmitCommandBuffer_Graphics(commandBuffer, semaphore_imageIsAvailable, semaphore_renderingIsOver, fence);
		graphicsBase::Base().PresentImage(semaphore_renderingIsOver);

		glfwPollEvents();
		TitleFps();

		fence.WaitAndReset();
	}
	TerminateWindow();

	
	return 0;
}