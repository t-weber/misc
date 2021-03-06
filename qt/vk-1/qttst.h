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
#include <QMatrix4x4>
#include <QVector4D>
#include <QVector3D>
#include <QTimer>
#include <QMouseEvent>

#include <memory>

#include "../../libs/math_algos.h"
#include "../../libs/math_conts.h"


using t_real = float;
using t_vec3 = m::qvecN_adapter<int, 3, t_real, QVector3D>;
using t_vec = m::qvecN_adapter<int, 4, t_real, QVector4D>;
using t_mat = m::qmatNN_adapter<int, 4, 4, t_real, QMatrix4x4>;


class VkWnd;


class VkRenderer : public QVulkanWindowRenderer
{
protected:
	std::shared_ptr<QVulkanInstance> m_vkinst;
	VkWnd* m_vkwnd = nullptr;
	VkDevice m_vkdev;
	QVulkanDeviceFunctions* m_vkfuncs = nullptr;

	VkShaderModule m_fragShader{VK_NULL_HANDLE};
	VkShaderModule m_vertexShader{VK_NULL_HANDLE};

	VkBuffer m_buffer{VK_NULL_HANDLE};

	VkPipeline m_pipeline{VK_NULL_HANDLE};
	VkPipelineLayout m_layout{VK_NULL_HANDLE};
	VkPipelineCache m_cache{VK_NULL_HANDLE};
	VkDescriptorSetLayout m_setlayouts[1] {{ VK_NULL_HANDLE }};

	t_mat m_matPerspective{m::unit<t_mat>(4)}, m_matPerspective_inv{m::unit<t_mat>(4)};
	t_mat m_matViewport{m::unit<t_mat>(4)}, m_matViewport_inv{m::unit<t_mat>(4)};
	t_mat m_matCam{m::unit<t_mat>(4)}, m_matCam_inv{m::unit<t_mat>(4)};

	int m_iScreenDims[2] = { -1, -1 };

protected:
	QPointF VkToScreenCoords(const t_vec& vec, bool *pVisible=nullptr);

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

	QPointF m_posMouse;
	QTimer m_timer;

public:
	VkWnd(std::shared_ptr<QVulkanInstance>& vk, QWindow* parent=nullptr);
	virtual ~VkWnd();

	virtual QVulkanWindowRenderer* createRenderer() override;

	virtual void mouseMoveEvent(QMouseEvent *pEvt) override;
};


#endif
