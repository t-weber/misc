/**
 * minimal qt vk example
 * @author Tobias Weber
 * @date Feb-2021
 * @license: see 'LICENSE.GPL' file
 *
 * References:
 *  * https://doc.qt.io/qt-5/qvulkanwindow.html
 *  * https://doc.qt.io/qt-5/qvulkaninstance.html
 *  * https://doc.qt.io/qt-5/qvulkanwindowrenderer.html
 *  * https://doc.qt.io/qt-5/qtgui-hellovulkanwindow-example.html
 *  * https://code.qt.io/cgit/qt/qtbase.git/tree/examples/vulkan/shared/trianglerenderer.cpp
 *  * https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkRenderPassBeginInfo.html
 *  * https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateShaderModule.html
 *  * https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#shaders
 */

#include "qttst.h"

#include <QApplication>
#include <QLoggingCategory>

#include <cstddef>
#include <locale>
#include <iostream>
#include <random>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

#include <boost/scope_exit.hpp>


static inline std::string get_vk_error(VkResult res)
{
	switch(res)
	{
		case VK_SUCCESS: return "operation successful";
		case VK_ERROR_OUT_OF_HOST_MEMORY: return "out of host memory";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "out of device memory";
		case VK_ERROR_INVALID_SHADER_NV: return "invalid shader";
		default: return "<unknown error code>";
	}
}


static inline std::string get_device_type(const VkPhysicalDeviceType& ty)
{
	switch(ty)
	{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "integrated gpu";
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "discrete gpu";
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "virtual gpu";
		case VK_PHYSICAL_DEVICE_TYPE_CPU: return "virtual cpu";
		case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "other";
		default: return "<unknown>";
	}
}


// ----------------------------------------------------------------------------
// vk renderer
// ----------------------------------------------------------------------------

VkRenderer::VkRenderer(std::shared_ptr<QVulkanInstance>& vk, VkWnd* wnd)
	: m_vkinst{vk}, m_vkwnd{wnd}
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}


VkRenderer::~VkRenderer()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}


QPointF VkRenderer::VkToScreenCoords(const t_vec& vec4, bool *pVisible)
{
	auto [ vecPersp, vec ] =
	m::hom_to_screen_coords<t_mat, t_vec>(vec4, m_matCam, m_matPerspective, m_matViewport, true);

	// position not visible -> return a point outside the viewport
	if(vecPersp[2] > 1.)
	{
		if(pVisible) *pVisible = false;
		return QPointF(-1*m_iScreenDims[0], -1*m_iScreenDims[1]);
	}

	if(pVisible) *pVisible = true;
	return QPointF(vec[0], vec[1]);
}


void VkRenderer::tick(const std::chrono::milliseconds& ms)
{
	//std::cout << ms.count() << std::endl;

	if(m_vkwnd)
		m_vkwnd->requestUpdate();
}


void VkRenderer::preInitResources()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}


void VkRenderer::initResources()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	const VkPhysicalDeviceProperties* props = m_vkwnd->physicalDeviceProperties();
	std::cout << "physical device:"
		<< "\n\tapi = " << props->apiVersion << ","
		<< "\n\tdriver = " << props->driverVersion << ","
		<< "\n\tvendor = " << props->vendorID << ","
		<< "\n\tdevice = " << props->deviceID << ","
		<< "\n\tname = " << props->deviceName << ","
		<< "\n\ttype = " << get_device_type(props->deviceType) << "."
		<< std::endl;

	m_vkdev = m_vkwnd->device();
	m_vkfuncs = m_vkinst->deviceFunctions(m_vkdev);


	// --------------------------------------------------------------------
	// shaders
	// --------------------------------------------------------------------
	if(!fs::exists("vert.spv") || !fs::exists("frag.spv"))
	{
		std::cerr << "Vertex or fragment shader could be found." << std::endl;
		return;
	}

	// load shaders
	using t_shader = std::tuple<const std::string, VkShaderModule*>;

	for(const t_shader& shader : {
		std::make_tuple("vert.spv", &m_vertexShader),
		std::make_tuple("frag.spv", &m_fragShader) })
	{
		const std::string& file = std::get<0>(shader);

		std::size_t size = fs::file_size(file);
		std::cout << "Loading shader " << file << ", size = " << size << "." << std::endl;

		std::vector<std::byte> bin;	// shader binary data
		bin.resize(size);
		if(std::ifstream ifstr{file}; !ifstr.read(reinterpret_cast<char*>(bin.data()), size))
		{
			std::cerr << "Error loading shader " << file << "." << std::endl;
			continue;
		}

		VkShaderModuleCreateInfo shaderInfo
		{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // fixed value
			.pNext = nullptr,
			.flags = 0, // unused
			.codeSize = size,
			.pCode = reinterpret_cast<decltype(VkShaderModuleCreateInfo::pCode)>(bin.data())
		};

		if(VkResult err = m_vkfuncs->vkCreateShaderModule(m_vkdev, &shaderInfo, 0, std::get<1>(shader));
		   err != VK_SUCCESS)
		{
			std::cerr << "Error compiling " << file <<  ": " << get_vk_error(err) << std::endl;
			continue;
		}
	}
	// --------------------------------------------------------------------


	// --------------------------------------------------------------------
	// pipeline stages
	// --------------------------------------------------------------------
	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineShaderStageCreateInfo.html
	VkPipelineShaderStageCreateInfo shaderstages[2] =
	{
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = m_vertexShader,
			.pName = "main",
			.pSpecializationInfo = nullptr,
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = m_fragShader,
			.pName = "main",
			.pSpecializationInfo = nullptr,
		},
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineVertexInputStateCreateInfo.html
	VkPipelineVertexInputStateCreateInfo vertexinputstate
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = nullptr,
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = nullptr,
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineInputAssemblyStateCreateInfo.html
	VkPipelineInputAssemblyStateCreateInfo inputassemblystate
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = 0,
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineTessellationStateCreateInfo.html
	VkPipelineTessellationStateCreateInfo tessellationstate
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.patchControlPoints = 0,
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineViewportStateCreateInfo.html
	VkPipelineViewportStateCreateInfo viewportstate
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.viewportCount = 1,
		.pViewports = nullptr,	/* nullptr => dynamically set */
		.scissorCount = 1,
		.pScissors = nullptr,	/* nullptr => dynamically set */
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineRasterizationStateCreateInfo.html
	VkPipelineRasterizationStateCreateInfo rasterisationstate
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthClampEnable = 0,
		.rasterizerDiscardEnable = 0,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = 0,
		.depthBiasConstantFactor = 0.f,
		.depthBiasClamp = 0.f,
		.depthBiasSlopeFactor = 0.f,
		.lineWidth = 1.f,
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineMultisampleStateCreateInfo.html
	VkPipelineMultisampleStateCreateInfo multisamplestate
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.rasterizationSamples = m_vkwnd->sampleCountFlagBits(),
		.sampleShadingEnable = 0,
		.minSampleShading = 0.f,
		.pSampleMask = nullptr,
		.alphaToCoverageEnable = 0,
		.alphaToOneEnable = 0,
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineDepthStencilStateCreateInfo.html
	VkPipelineDepthStencilStateCreateInfo depthstencilstate
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthTestEnable = 1,
		.depthWriteEnable = 1,
		.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
		.depthBoundsTestEnable = 0,
		.stencilTestEnable = 0,
		.front = VkStencilOpState
		{
			.failOp = VK_STENCIL_OP_KEEP,
			.passOp = VK_STENCIL_OP_KEEP,
			.depthFailOp = VK_STENCIL_OP_KEEP,
			.compareOp = VK_COMPARE_OP_NEVER,
			.compareMask = 0,
			.writeMask = 0,
			.reference = 0,
		},
		.back = VkStencilOpState
		{
			.failOp = VK_STENCIL_OP_KEEP,
			.passOp = VK_STENCIL_OP_KEEP,
			.depthFailOp = VK_STENCIL_OP_KEEP,
			.compareOp = VK_COMPARE_OP_NEVER,
			.compareMask = 0,
			.writeMask = 0,
			.reference = 0,
		},
		.minDepthBounds = 0.f,
		.maxDepthBounds = 0.f,
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineColorBlendAttachmentState.html
	VkPipelineColorBlendAttachmentState colorblendattachments[]
	{
		{
			.blendEnable = 0,
			.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		},
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineColorBlendStateCreateInfo.html
	VkPipelineColorBlendStateCreateInfo colorblendstate
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.logicOpEnable = 0,
		.logicOp = VK_LOGIC_OP_CLEAR,
		.attachmentCount = sizeof(colorblendattachments)/sizeof(colorblendattachments[0]),
		.pAttachments = colorblendattachments,
		.blendConstants = {0.f, 0.f, 0.f, 0.f},
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkDynamicState.html
	VkDynamicState dynstate[] = {{ VK_DYNAMIC_STATE_VIEWPORT }, { VK_DYNAMIC_STATE_SCISSOR} };

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineDynamicStateCreateInfo.html
	VkPipelineDynamicStateCreateInfo dynamicstate
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.dynamicStateCount = sizeof(dynstate)/sizeof(dynstate[0]),
		.pDynamicStates = dynstate,
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkDescriptorSetLayoutBinding.html
	VkDescriptorSetLayoutBinding setlayoutbindings[]
	{
		{
			.binding = 0,
			// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkDescriptorType.html
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
			.descriptorCount = 0,
			// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkShaderStageFlagBits.html
			.stageFlags = 0,
			.pImmutableSamplers = nullptr,
		},
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkDescriptorSetLayoutCreateInfo.html
	VkDescriptorSetLayoutCreateInfo setlayoutinfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.bindingCount = sizeof(setlayoutbindings) / sizeof(setlayoutbindings[0]),
		.pBindings = setlayoutbindings,
	};

	if(VkResult err = m_vkfuncs->vkCreateDescriptorSetLayout(m_vkdev, &setlayoutinfo, 0, &m_setlayouts[0]);
	   err != VK_SUCCESS)
	{
		std::cerr << "Error creating set layout: " << get_vk_error(err) << std::endl;
		return;
	}

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPushConstantRange.html
	VkPushConstantRange pushconstrange[] = {};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineLayoutCreateInfo.html
	VkPipelineLayoutCreateInfo layoutcreateinfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = sizeof(m_setlayouts)/sizeof(m_setlayouts[0]),
		.pSetLayouts = m_setlayouts,
		.pushConstantRangeCount = sizeof(pushconstrange)/sizeof(pushconstrange[0]),
		.pPushConstantRanges = pushconstrange,
	};

	if(VkResult err = m_vkfuncs->vkCreatePipelineLayout(m_vkdev, &layoutcreateinfo, 0, &m_layout);
		err != VK_SUCCESS)
	{
		std::cerr << "Error creating graphics pipeline layout: " << get_vk_error(err) << std::endl;
		return;
	}

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkGraphicsPipelineCreateInfo.html
	VkGraphicsPipelineCreateInfo createInfos
	{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stageCount = sizeof(shaderstages)/sizeof(shaderstages[0]),
		.pStages = shaderstages,
		.pVertexInputState = &vertexinputstate,
		.pInputAssemblyState = &inputassemblystate,
		.pTessellationState = &tessellationstate,
		.pViewportState = &viewportstate,
		.pRasterizationState = &rasterisationstate,
		.pMultisampleState = &multisamplestate,
		.pDepthStencilState = &depthstencilstate,
		.pColorBlendState = &colorblendstate,
		.pDynamicState = &dynamicstate,
		.layout = m_layout,
		.renderPass = m_vkwnd->defaultRenderPass(),
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = 0,
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineCacheCreateInfo.html
	VkPipelineCacheCreateInfo cachecreateinfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.initialDataSize = 0,
		.pInitialData = nullptr,
	};

	if(VkResult err = m_vkfuncs->vkCreatePipelineCache(m_vkdev, &cachecreateinfo, 0, &m_cache);
	   err != VK_SUCCESS)
	{
		std::cerr << "Error creating graphics pipeline cache: " << get_vk_error(err) << std::endl;
		return;
	}

	if(VkResult err = m_vkfuncs->vkCreateGraphicsPipelines(m_vkdev, m_cache, 1, &createInfos, nullptr, &m_pipeline);
		err != VK_SUCCESS)
	{
		std::cerr << "Error creating graphics pipeline: " << get_vk_error(err) << std::endl;
	}
	// --------------------------------------------------------------------
}


void VkRenderer::releaseResources()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;

	// handles
	VK_DEFINE_NON_DISPATCHABLE_HANDLE(vkhandle);
	vkhandle handles[] =
	{
		// shaders
		reinterpret_cast<vkhandle>(m_fragShader),
		reinterpret_cast<vkhandle>(m_vertexShader),

		// set layouts
		reinterpret_cast<vkhandle>(m_setlayouts[0]),

		// pipeline
		reinterpret_cast<vkhandle>(m_cache),
		reinterpret_cast<vkhandle>(m_layout),
		reinterpret_cast<vkhandle>(m_pipeline),
	};

	// functions to destroy respective handles
	using t_destroyfunc = void (QVulkanDeviceFunctions::*)(VkDevice, vkhandle, const VkAllocationCallbacks*);
	t_destroyfunc destroyfuncs[] =
	{
		// shaders
		reinterpret_cast<t_destroyfunc>(&QVulkanDeviceFunctions::vkDestroyShaderModule),
		reinterpret_cast<t_destroyfunc>(&QVulkanDeviceFunctions::vkDestroyShaderModule),

		// set layouts
		reinterpret_cast<t_destroyfunc>(&QVulkanDeviceFunctions::vkDestroyDescriptorSetLayout),

		// pipeline
		reinterpret_cast<t_destroyfunc>(&QVulkanDeviceFunctions::vkDestroyPipelineCache),
		reinterpret_cast<t_destroyfunc>(&QVulkanDeviceFunctions::vkDestroyPipelineLayout),
		reinterpret_cast<t_destroyfunc>(&QVulkanDeviceFunctions::vkDestroyPipeline),
	};

	for(std::size_t idx=0; idx<sizeof(handles)/sizeof(handles[0]); ++idx)
	{
		if(handles[idx] == VK_NULL_HANDLE)
			continue;

		(m_vkfuncs->*destroyfuncs[idx])(m_vkdev, handles[idx], 0);
		handles[idx] = VK_NULL_HANDLE;
	}

	m_vkfuncs = nullptr;
}


void VkRenderer::initSwapChainResources()
{
	using namespace m_ops;
	std::cout << __PRETTY_FUNCTION__ << std::endl;

	m_iScreenDims[0] = m_vkwnd->swapChainImageSize().width();
	m_iScreenDims[1] = m_vkwnd->swapChainImageSize().height();
	std::cout << "window size: " << m_iScreenDims[0] << " x " << m_iScreenDims[1] << "." << std::endl;

	m_matViewport = m::hom_viewport<t_mat>(m_iScreenDims[0], m_iScreenDims[1], 0., 1.);
	std::tie(m_matViewport_inv, std::ignore) = m::inv<t_mat>(m_matViewport);

	m_matPerspective = m::hom_perspective<t_mat>(
		0.01, 100., m::pi<t_real>*0.5,
		t_real(m_iScreenDims[1])/t_real(m_iScreenDims[0]), false, true, true);
	std::tie(m_matPerspective_inv, std::ignore) = m::inv<t_mat>(m_matPerspective);
	std::cout << "perspective matrix: " << m_matPerspective << "." << std::endl;
	std::cout << "inverted perspective matrix: " << m_matPerspective_inv << "." << std::endl;
}


void VkRenderer::releaseSwapChainResources()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}


void VkRenderer::logicalDeviceLost()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}


void VkRenderer::physicalDeviceLost()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}


void VkRenderer::startNextFrame()
{
	//std::cout << __PRETTY_FUNCTION__ << std::endl;

	VkClearValue clr[] /* union between .color and .depthStencil */
	{
		{ .color = VkClearColorValue{.float32 = {1.f, 1.f, 1.f, 1.f}} },
		{ .depthStencil = VkClearDepthStencilValue{.depth = 1.f, .stencil = 0} }
	};

	VkRenderPassBeginInfo beg
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,	// fixed value
		.pNext = nullptr,
		.renderPass = m_vkwnd->defaultRenderPass(),
		.framebuffer = m_vkwnd->currentFramebuffer(),
		.renderArea = VkRect2D
		{
			.offset = VkOffset2D{.x = 0, .y = 0},
			.extent = VkExtent2D
			{
				.width = (uint32_t)m_vkwnd->swapChainImageSize().width(),
				.height = (uint32_t)m_vkwnd->swapChainImageSize().height()
			},
		},
		.clearValueCount = sizeof(clr) / sizeof(clr[0]),
		.pClearValues = clr
	};

	VkSubpassContents cont = VK_SUBPASS_CONTENTS_INLINE /*VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS*/;
	m_vkfuncs->vkCmdBeginRenderPass(m_vkwnd->currentCommandBuffer(), &beg, cont);
	m_vkfuncs->vkCmdEndRenderPass(m_vkwnd->currentCommandBuffer());

	m_vkwnd->frameReady();
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// vk window
// ----------------------------------------------------------------------------

VkWnd::VkWnd(std::shared_ptr<QVulkanInstance>& vk, QWindow* parent)
	: QVulkanWindow{parent}, m_vkinst{vk}
{
	setVulkanInstance(m_vkinst.get());

	connect(&m_timer, &QTimer::timeout,
		[this]() -> void
		{
			if(m_vkrenderer)
				m_vkrenderer->tick(std::chrono::milliseconds(1000 / 60));
		});

	m_timer.start(std::chrono::milliseconds(1000 / 60));
}


VkWnd::~VkWnd()
{
	m_timer.stop();
}


QVulkanWindowRenderer* VkWnd::createRenderer()
{
	if(m_vkrenderer)
		delete m_vkrenderer;

	return m_vkrenderer = new VkRenderer(m_vkinst, this);
}


void VkWnd::mouseMoveEvent(QMouseEvent *pEvt)
{
	m_posMouse = pEvt->localPos();
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// init
// ----------------------------------------------------------------------------

static inline void set_locales()
{
	std::ios_base::sync_with_stdio(false);

	::setlocale(LC_ALL, "C");
	std::locale::global(std::locale("C"));
	QLocale::setDefault(QLocale::C);
}


int main(int argc, char** argv)
{
	QLoggingCategory::setFilterRules("*=true\n*.debug=true\n");
	qInstallMessageHandler([](QtMsgType ty, const QMessageLogContext& ctx, const QString& log) -> void
	{
		auto get_msg_type = [](const QtMsgType& _ty) -> std::string
		{
			switch(_ty)
			{
				case QtDebugMsg: return "debug";
				case QtWarningMsg: return "warning";
				case QtCriticalMsg: return "critical";
				case QtFatalMsg: return "fatal";
				case QtInfoMsg: return "info";
				default: return "<unknown>";
			}
		};

		auto get_str = [](const char* pc) -> std::string
		{
			if(!pc) return "<unknown>";
			return std::string{"\""} + std::string{pc} + std::string{"\""};
		};

		std::cerr << "qt " << get_msg_type(ty);
		if(ctx.function)
		{
			std::cerr << " in "
				<< "file " << get_str(ctx.file) << ", "
				<< "function " << get_str(ctx.function) << ", "
				<< "line " << ctx.line;
		}
		std::cerr << ": " << log.toStdString() << std::endl;
	});

	auto app = std::make_unique<QApplication>(argc, argv);
	set_locales();

	// create vk instance
	auto vk = std::make_shared<QVulkanInstance>();

	QByteArrayList layers{{
		"VK_LAYER_KHRONOS_validation",
		"VK_EXT_debug_report",
		"VK_EXT_debug_utils",
	}};
	vk->setLayers(layers);
	vk->setFlags(vk->flags() & ~QVulkanInstance::NoDebugOutputRedirect);

	if(!vk->create() || !vk->isValid())
	{
		std::cerr << "Cannot create a valid Vk instance." << std::endl;
		return -1;
	}

	// get version infos
	std::string vkver = vk->apiVersion().toString().toStdString();
	if(vkver != "")
		std::cout << "Vk API version: " << vkver << "." << std::endl;

	// get layer infos
	QVulkanInfoVector<QVulkanLayer> vklayers = vk->supportedLayers();
	for(const auto& vklayer : vklayers)
	{
		std::cout << "Vk layer: " << vklayer.name.toStdString()
			<< ", description: " << vklayer.description.toStdString()
			<< ", layer version: " << vklayer.version
			<< ", Vk version: " << vklayer.specVersion.toString().toStdString()
			<< "." << std::endl;
	}

	// get extension infos
	QVulkanInfoVector<QVulkanExtension> vkexts = vk->supportedExtensions();
	for(const auto& vkext : vkexts)
	{
		std::cout << "Vk extension: " << vkext.name.toStdString()
			<< ", version " << vkext.version << "." << std::endl;
	}

	// create vk window
	auto wnd = std::make_unique<VkWnd>(vk);
	wnd->resize(800, 600);
	wnd->show();

	// run application
	int ret = app->exec();
	vk->destroy();
	return ret;
}
// ----------------------------------------------------------------------------
