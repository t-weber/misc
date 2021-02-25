/**
 * minimal qt vk example
 * @author Tobias Weber
 * @date Feb-2021
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __QTVKTST_H__
#define __QTVKTST_H__

#include <QVulkanWindow>
#include <QVulkanWindowRenderer>
#include <QVulkanDeviceFunctions>
#include <QTimer>

#include <memory>


class VkWnd;


class VkRenderer : public QVulkanWindowRenderer
{
protected:
	std::shared_ptr<QVulkanInstance> m_vkinst;
	VkWnd* m_vkwnd = nullptr;
	VkDevice m_vkdev;
	QVulkanDeviceFunctions* m_vkfuncs = nullptr;

protected:
	float m_col[3] = { 0.f, 0.f, 0.f };
	bool m_coldir[3] = {1, 1, 1};

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

	void tick(const std::chrono::milliseconds& ms);
};


class VkWnd : public QVulkanWindow
{
protected:
	std::shared_ptr<QVulkanInstance> m_vkinst;
	VkRenderer* m_vkrenderer = nullptr;

	QTimer m_timer;

protected:

public:
	VkWnd(std::shared_ptr<QVulkanInstance>& vk, QWindow* parent=nullptr);
	virtual ~VkWnd();

	virtual QVulkanWindowRenderer* createRenderer() override;
};


#endif
