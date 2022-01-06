/**
 * vk renderer
 * @author Tobias Weber
 * @date Feb-2021
 * @license see 'LICENSE.GPL' file
 */

#ifndef __QTVKTST_RENDERER_H__
#define __QTVKTST_RENDERER_H__

#include <QVulkanWindowRenderer>
#include <QVulkanDeviceFunctions>

#include <memory>

#include "../../libs/math_algos.h"
#include "../../libs/math_conts.h"
#include "cam.h"
#include "viewport.h"
#include "obj.h"


class VkWnd;


class VkRenderer : public QVulkanWindowRenderer
{
private:
	QPointF m_posMouse{};
	std::vector<PolyObject> m_objs;
	VkBuffer m_buffer{VK_NULL_HANDLE};

protected:
	std::shared_ptr<QVulkanInstance> m_vkinst;
	VkWnd* m_vkwnd = nullptr;
	VkDevice m_vkdev;
	QVulkanDeviceFunctions* m_vkfuncs = nullptr;

	VkShaderModule m_fragShader{VK_NULL_HANDLE};
	VkShaderModule m_vertexShader{VK_NULL_HANDLE};

	VkDeviceMemory m_mem{VK_NULL_HANDLE};
	std::size_t m_bufferoffsetgranularity{256};

	VkPipeline m_pipeline{VK_NULL_HANDLE};
	VkPipelineLayout m_layout{VK_NULL_HANDLE};
	VkPipelineCache m_cache{VK_NULL_HANDLE};
	VkDescriptorSetLayout m_setlayouts[1] { VK_NULL_HANDLE };
	VkDescriptorPool m_descrpool{VK_NULL_HANDLE};
	std::shared_ptr<VkDescriptorSet[]> m_descrset;
	std::shared_ptr<VkDescriptorBufferInfo[]> m_descrbufferinfo;

	t_vec2 m_veccurUV = m::create<t_vec2>({ 0.f, 0.f });
	Camera<t_mat, t_vec, t_real> m_cam;
	Viewport<t_mat, t_vec, t_real> m_viewport;

	t_real m_moving[3] = {0., 0., 0.};
	t_real m_rotating[3] = {0., 0., 0.};

	VkViewport m_viewports[1];
	VkRect2D m_viewrects[1];

protected:
	void UpdatePicker();
	void UpdateVertexBuffers();
	void UpdateUniforms();

	std::size_t GetNumShaderInputElements() const;
	std::size_t GetUniformBufferSize(bool use_granularity=false) const;
	std::size_t GetFullSizeVertexBuffer(bool use_granularity=true) const;
	std::size_t GetFullSizeUniformBuffer(bool use_granularity=true) const;

	std::vector<VkPipelineShaderStageCreateInfo> CreateShaders();
	void CreateBuffers();
	void CreatePipelineLayout();
	void CreatePipelineCache();
	std::tuple<VkPipelineInputAssemblyStateCreateInfo, VkPipelineTessellationStateCreateInfo,
		VkPipelineViewportStateCreateInfo, VkPipelineRasterizationStateCreateInfo,
		VkPipelineMultisampleStateCreateInfo, VkPipelineDepthStencilStateCreateInfo>
	CreatePipelineStages() const;

public:
	VkRenderer(std::shared_ptr<QVulkanInstance>& vk, VkWnd* wnd);
	virtual ~VkRenderer();

	std::size_t AddObject(const PolyObject& obj);
	PolyObject* GetObject(std::size_t idx) { return idx < m_objs.size() ? &m_objs[idx] : nullptr; }
	const PolyObject* GetObject(std::size_t idx) const { return idx < m_objs.size() ? &m_objs[idx] : nullptr; }

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

	Camera<t_mat, t_vec, t_real>& GetCamera() { return m_cam; }
	void SetMoving(std::size_t axis, t_real val) { m_moving[axis] = val; }
	void SetRotating(std::size_t axis, t_real val) { m_rotating[axis] = val; }
};

#endif
