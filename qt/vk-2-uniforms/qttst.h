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
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QVector4D>
#include <QVector3D>
#include <QVector2D>
#include <QTimer>
#include <QMouseEvent>

#include <memory>

#include "../../libs/math_algos.h"
#include "../../libs/math_conts.h"


using t_real = float;
using t_vec2 = m::qvecN_adapter<int, 2, t_real, QVector2D>;
using t_vec3 = m::qvecN_adapter<int, 3, t_real, QVector3D>;
using t_vec = m::qvecN_adapter<int, 4, t_real, QVector4D>;
using t_mat = m::qmatNN_adapter<int, 4, 4, t_real, QMatrix4x4>;
using t_mat3 = m::qmatNN_adapter<int, 3, 3, t_real, QMatrix3x3>;


class VkWnd;


class VkRenderer : public QVulkanWindowRenderer
{
private:
	std::vector<t_real> m_vecVerts, m_vecNorms, m_vecCols, m_vecUVs;
	std::vector<t_vec3> m_triangles, m_trianglenorms, m_triangleuvs;

	QPointF m_posMouse{};

protected:
	std::shared_ptr<QVulkanInstance> m_vkinst;
	VkWnd* m_vkwnd = nullptr;
	VkDevice m_vkdev;
	QVulkanDeviceFunctions* m_vkfuncs = nullptr;

	VkShaderModule m_fragShader{VK_NULL_HANDLE};
	VkShaderModule m_vertexShader{VK_NULL_HANDLE};

	VkBuffer m_buffer{VK_NULL_HANDLE};
	VkDeviceMemory m_mem{VK_NULL_HANDLE};
	std::size_t m_bufferoffsetgranularity{256};

	VkPipeline m_pipeline{VK_NULL_HANDLE};
	VkPipelineLayout m_layout{VK_NULL_HANDLE};
	VkPipelineCache m_cache{VK_NULL_HANDLE};
	VkDescriptorSetLayout m_setlayouts[1] {{ VK_NULL_HANDLE }};
	VkDescriptorPool m_descrpool{VK_NULL_HANDLE};
	std::shared_ptr<VkDescriptorSet[]> m_descrset;
	std::shared_ptr<VkDescriptorBufferInfo[]> m_descrbufferinfo;

	t_mat m_matPerspective{m::unit<t_mat>(4)}, m_matPerspective_inv{m::unit<t_mat>(4)};
	t_mat m_matViewport{m::unit<t_mat>(4)}, m_matViewport_inv{m::unit<t_mat>(4)};
	t_mat m_matCam{m::unit<t_mat>(4)}, m_matCam_inv{m::unit<t_mat>(4)};
	t_vec2 m_veccurUV = m::create<t_vec2>({ 0.f, 0.f });

	VkViewport m_viewports[1];
	VkRect2D m_viewrects[1];
	std::uint32_t m_iScreenDims[2] = { 800, 600 };
	bool m_use_prespective_proj = true;

protected:
	QPointF VkToScreenCoords(const t_vec& vec, bool *pVisible=nullptr);

	void UpdatePicker();
	void UpdateVertexBuffers();
	void UpdateUniforms();

	std::size_t GetNumShaderInputElements() const;
	std::size_t GetNumVertexBufferElements() const;
	std::size_t GetNumUniformBufferElements() const;
	std::size_t GetSizeVertexBuffer(bool use_granularity=true) const;
	std::size_t GetSizeUniformBuffer(bool use_granularity=true) const;

	std::vector<VkPipelineShaderStageCreateInfo> CreateShaders();
	void CreateGeometry();
	void CreateBuffers();
	void CreatePipelineLayout();
	void CreatePipelineCache();
	std::tuple<VkPipelineInputAssemblyStateCreateInfo, VkPipelineTessellationStateCreateInfo,
		VkPipelineViewportStateCreateInfo, VkPipelineRasterizationStateCreateInfo,
		VkPipelineMultisampleStateCreateInfo, VkPipelineDepthStencilStateCreateInfo>
	CreatePipelineStages() const;

	void UpdatePerspective();

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

	void TogglePerspective();
	void SetMousePos(const QPointF& pt);
	void tick(const std::chrono::milliseconds& ms);
};


class VkWnd : public QVulkanWindow
{
protected:
	std::shared_ptr<QVulkanInstance> m_vkinst;
	VkRenderer* m_vkrenderer = nullptr;

	QTimer m_timer;

public:
	VkWnd(std::shared_ptr<QVulkanInstance>& vk, QWindow* parent=nullptr);
	virtual ~VkWnd();

	virtual QVulkanWindowRenderer* createRenderer() override;

	virtual void mouseMoveEvent(QMouseEvent *pEvt) override;
	virtual void keyPressEvent(QKeyEvent *pEvt) override;
};


#endif
