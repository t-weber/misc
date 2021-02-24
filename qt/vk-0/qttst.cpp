/**
 * minimal qt vk example
 * @author Tobias Weber
 * @date feb-2021
 * @license: see 'LICENSE.GPL' file
 *
 * References:
 *  * https://doc.qt.io/qt-5/qvulkanwindow.html
 *  * https://doc.qt.io/qt-5/qvulkaninstance.html
 *  * https://doc.qt.io/qt-5/qvulkanwindowrenderer.html
 */

#include "qttst.h"

#include <QApplication>

#include <locale>
#include <iostream>

#include <boost/scope_exit.hpp>
#include <boost/algorithm/string/replace.hpp>
namespace algo = boost::algorithm;



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


void VkRenderer::preInitResources()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}


void VkRenderer::initResources()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;

	m_vkdev = m_vkwnd->device();
	m_vkfuncs = m_vkinst->deviceFunctions(m_vkdev);
}


void VkRenderer::releaseResources()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;

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
	std::cout << __PRETTY_FUNCTION__ << std::endl;

	m_vkwnd->frameReady();
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// vk window
// ----------------------------------------------------------------------------

VkWnd::VkWnd(std::shared_ptr<QVulkanInstance>& vk, QWindow* parent)
	: m_vkinst{vk}, QVulkanWindow(parent)
{
	setVulkanInstance(m_vkinst.get());

	QMatrix4x4 m = clipCorrectionMatrix();
	std::cout << "Gl -> Vk: \n"
		<< m(0,0) << " " << m(0,1) << " " << m(0,2) << " " << m(0,3) << "\n"
		<< m(1,0) << " " << m(1,1) << " " << m(1,2) << " " << m(1,3) << "\n"
		<< m(2,0) << " " << m(2,1) << " " << m(2,2) << " " << m(2,3) << "\n"
		<< m(3,0) << " " << m(3,1) << " " << m(3,2) << " " << m(3,3) << "\n"
		<< std::endl;
}


VkWnd::~VkWnd()
{
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
