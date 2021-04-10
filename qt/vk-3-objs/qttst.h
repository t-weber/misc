/**
 * minimal qt vk example
 * @author Tobias Weber
 * @date Feb-2021
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __QTVKTST_H__
#define __QTVKTST_H__

#include <QMainWindow>
#include <QStatusBar>
#include <QLabel>
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
#include "cam.h"


using t_real = float;
using t_vec2 = m::qvecN_adapter<int, 2, t_real, QVector2D>;
using t_vec3 = m::qvecN_adapter<int, 3, t_real, QVector3D>;
using t_vec = m::qvecN_adapter<int, 4, t_real, QVector4D>;
using t_mat = m::qmatNN_adapter<int, 4, 4, t_real, QMatrix4x4>;
using t_mat3 = m::qmatNN_adapter<int, 3, 3, t_real, QMatrix3x3>;


class VkWnd;


class PolyObject
{
private:
	std::vector<t_real> m_vecVerts, m_vecNorms, m_vecCols, m_vecUVs;
	std::vector<t_vec3> m_triangles, m_trianglenorms, m_triangleuvs;
	t_mat m_mat{m::unit<t_mat>(4)};
	std::size_t m_mem_offs = 0;
	bool m_rotating = false;

public:
	std::size_t GetNumVertexBufferElements() const;
	std::size_t GetNumVertices() const;

	const t_vec3& GetVertex(std::size_t i) const;
	const t_vec3& GetUV(std::size_t i) const;

	void CreatePlaneGeometry(
		const t_vec3& norm=m::create<t_vec3>({0,0,-1}), t_real size=1.5,
		t_real r=0, t_real g=0, t_real b=1);
	void CreateCubeGeometry(t_real size=1., t_real r=0, t_real g=0, t_real b=1);

	std::size_t UpdateVertexBuffers(t_real* pMem, std::size_t mem_offs);
	std::size_t GetMemOffset() const;

	void SetMatrix(const t_mat& mat);
	const t_mat& GetMatrix() const;

	void SetRotating(bool b);

	void tick(const std::chrono::milliseconds& ms);
};


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
	VkDescriptorSetLayout m_setlayouts[1] {{ VK_NULL_HANDLE }};
	VkDescriptorPool m_descrpool{VK_NULL_HANDLE};
	std::shared_ptr<VkDescriptorSet[]> m_descrset;
	std::shared_ptr<VkDescriptorBufferInfo[]> m_descrbufferinfo;

	t_mat m_matPerspective{m::unit<t_mat>(4)}, m_matPerspective_inv{m::unit<t_mat>(4)};
	t_mat m_matViewport{m::unit<t_mat>(4)}, m_matViewport_inv{m::unit<t_mat>(4)};
	t_vec2 m_veccurUV = m::create<t_vec2>({ 0.f, 0.f });
	Camera<t_mat, t_vec, t_real> m_cam;
	t_real m_moving[3] = {0., 0., 0.};
	t_real m_rotating[3] = {0., 0., 0.};

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

	Camera<t_mat, t_vec, t_real>& GetCamera() { return m_cam; }
	void SetMoving(std::size_t axis, t_real val) { m_moving[axis] = val; }
	void SetRotating(std::size_t axis, t_real val) { m_rotating[axis] = val; }
};


class VkWnd : public QVulkanWindow
{ Q_OBJECT
protected:
	std::shared_ptr<QVulkanInstance> m_vkinst;
	VkRenderer *m_vkrenderer = nullptr;

	QTimer m_timer;
	std::chrono::milliseconds m_runningtime{0};

public:
	VkWnd(std::shared_ptr<QVulkanInstance>& vk, QWindow* parent=nullptr);
	virtual ~VkWnd();

	virtual QVulkanWindowRenderer* createRenderer() override;

	virtual void mouseMoveEvent(QMouseEvent *pEvt) override;
	virtual void keyPressEvent(QKeyEvent *pEvt) override;
	virtual void keyReleaseEvent(QKeyEvent *pEvt) override;

signals:
	void EmitStatusMsg(const QString& msg);
};


class Wnd : public QMainWindow
{
protected:
	VkWnd *m_vkwnd = nullptr;
	QWidget *m_vkwidget = nullptr;

	QStatusBar *m_statusbar = nullptr;
	QLabel *m_statuslabel = nullptr;

protected:
	virtual void resizeEvent(QResizeEvent *evt) override;

public:
	Wnd(VkWnd *vkwnd, QWidget *parent=nullptr);
	virtual ~Wnd();
};

#endif
