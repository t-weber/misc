/**
 * minimal qt vk example
 * @author Tobias Weber
 * @date Nov-2017
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __QTTST_H__
#define __QTTST_H__

#include <QVulkanWindow>
#include <QVulkanWindowRenderer>
#include <memory>


class VkWnd;


class VkRenderer : public QVulkanWindowRenderer
{
protected:
	std::shared_ptr<QVulkanInstance> m_vkinst;
	VkWnd* m_vkwnd = nullptr;
	VkDevice m_vkdev;
	QVulkanDeviceFunctions* m_vkfuncs = nullptr;

public:
	VkRenderer(std::shared_ptr<QVulkanInstance>& vk, VkWnd* wnd);
	virtual ~VkRenderer();

	virtual void preInitResources() override;
	virtual void initResources() override;
	virtual void releaseResources() override;

	virtual void initSwapChainResources() override;
	virtual void releaseSwapChainResources() override;

	virtual void logicalDeviceLost() override;
	virtual void physicalDeviceLost() override;

	virtual void startNextFrame() override;
};


class VkWnd : public QVulkanWindow
{
protected:
	std::shared_ptr<QVulkanInstance> m_vkinst;
	VkRenderer* m_vkrenderer = nullptr;

public:
	VkWnd(std::shared_ptr<QVulkanInstance>& vk, QWindow* parent=nullptr);
	virtual ~VkWnd();

	virtual QVulkanWindowRenderer* createRenderer() override;
};


#endif
