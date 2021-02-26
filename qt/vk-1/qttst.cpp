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

#include <locale>
#include <iostream>
#include <random>

#include <boost/scope_exit.hpp>
#include <boost/algorithm/string/replace.hpp>
namespace algo = boost::algorithm;


#define _GL_MAJ_VER 4
#define _GL_MIN_VER 5


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

	m_vkdev = m_vkwnd->device();
	m_vkfuncs = m_vkinst->deviceFunctions(m_vkdev);


	// --------------------------------------------------------------------
	// shaders
	// --------------------------------------------------------------------
	std::string strFragShader = R"RAW(
		#version ${GLSL_VERSION}

		in vec4 fragcolor;
		out vec4 outcolor;

		uniform sampler2D img;
		in vec2 fragtexcoords;

		// cursor position
		uniform vec2 fragCurUV = vec2(0.25, 0.25);

		void main()
		{
			outcolor = texture(img, fragtexcoords);
			outcolor *= fragcolor;

			// paint cursor position
			if(length(fragtexcoords - fragCurUV) < 0.01)
				outcolor = vec4(1,1,1,1);
		}
	)RAW";


	std::string strVertexShader = R"RAW(
		#version ${GLSL_VERSION}

		in vec4 vertex;
		in vec4 normal;
		in vec4 vertexcolor;
		out vec4 fragcolor;

		in vec2 texcoords;
		out vec2 fragtexcoords;

		uniform mat4 proj = mat4(1.);
		uniform mat4 cam = mat4(1.);

		vec3 light_dir = vec3(2, 2, -1);

		float lighting(vec3 lightdir)
		{
			float I = dot(vec3(cam*normal), normalize(lightdir));
			if(I < 0) I = 0;
			return I;
		}

		void main()
		{
			gl_Position = proj * cam * vertex;

			float I = lighting(light_dir);
			fragcolor = vertexcolor * I;
			fragcolor[3] = 1;

			fragtexcoords = texcoords;
		}
	)RAW";


	// set glsl version
	std::string strGlsl = std::to_string(_GL_MAJ_VER*100 + _GL_MIN_VER*10);
	for(std::string* strSrc : { &strFragShader, &strVertexShader })
		algo::replace_all(*strSrc, std::string("${GLSL_VERSION}"), strGlsl);


	// compile shaders
	using t_shader = std::tuple<const std::string*, VkShaderModule*, std::string>;

	for(const t_shader& shader : {
		std::make_tuple(&strFragShader, &m_fragShader, "fragment"),
		std::make_tuple(&strVertexShader, &m_vertexShader, "vertex") })
	{
		std::cout << "Compiling " << std::get<2>(shader) << " shader." << std::endl;

		VkShaderModuleCreateInfo shaderInfo
		{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // fixed value
			.pNext = nullptr,
			.flags = 0, // unused
			.codeSize = std::get<0>(shader)->length(),
			.pCode = reinterpret_cast<decltype(VkShaderModuleCreateInfo::pCode)>
				(std::get<0>(shader)->c_str())
		};

		if(VkResult err = m_vkfuncs->vkCreateShaderModule(m_vkdev, &shaderInfo, 0, std::get<1>(shader));
		   err != VK_SUCCESS)
		{
			std::cerr << "Error compiling " << std::get<2>(shader)
				<<  " shader: " << get_vk_error(err) << std::endl;
		}
	}
	// --------------------------------------------------------------------
}


void VkRenderer::releaseResources()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;

	// shader
	for(VkShaderModule* mod : {&m_fragShader, &m_vertexShader})
	{
		if(*mod != VK_NULL_HANDLE)
		{
			m_vkfuncs->vkDestroyShaderModule(m_vkdev, *mod, 0);
			*mod = VK_NULL_HANDLE;
		}
	}

	m_vkfuncs = nullptr;
}


void VkRenderer::initSwapChainResources()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
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

	VkClearValue clr /* union between .color and .depthStencil */
	{
		.color = VkClearColorValue{.float32 = {1.f, 1.f, 1.f, 1.f}},
		//.depthStencil = VkClearDepthStencilValue{.depth = 1.f, .stencil = 0}
	};

	VkRenderPassBeginInfo beg
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,	// fixed value
		.pNext = nullptr,
		.renderPass = m_vkwnd->defaultRenderPass(),
		.framebuffer = m_vkwnd->currentFramebuffer(),
		.renderArea = VkRect2D{
			.offset = VkOffset2D{.x = 0, .y = 0},
			.extent = VkExtent2D{
				.width = (uint32_t)m_vkwnd->swapChainImageSize().width(),
				.height = (uint32_t)m_vkwnd->swapChainImageSize().height()}},
		.clearValueCount = 1,
		.pClearValues = &clr
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

	QMatrix4x4 m = clipCorrectionMatrix();
	std::cout << "Gl -> Vk: \n"
		<< m(0,0) << " " << m(0,1) << " " << m(0,2) << " " << m(0,3) << "\n"
		<< m(1,0) << " " << m(1,1) << " " << m(1,2) << " " << m(1,3) << "\n"
		<< m(2,0) << " " << m(2,1) << " " << m(2,2) << " " << m(2,3) << "\n"
		<< m(3,0) << " " << m(3,1) << " " << m(3,2) << " " << m(3,3) << "\n"
		<< std::endl;

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
	auto app = std::make_unique<QApplication>(argc, argv);
	set_locales();

	// create vk instance
	auto vk = std::make_shared<QVulkanInstance>();

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
