/**
 * minimal qt vk example
 * @author Tobias Weber
 * @date Feb-2021
 * @license: see 'LICENSE.GPL' file
 *
 * References:
 *  * https://code.qt.io/cgit/qt/qtbase.git/tree/examples/vulkan/shared/trianglerenderer.cpp
 *  * https://doc.qt.io/qt-5/qvulkanwindow.html
 *  * https://doc.qt.io/qt-5/qvulkaninstance.html
 *  * https://doc.qt.io/qt-5/qvulkanwindowrenderer.html
 *  * https://doc.qt.io/qt-5/qtgui-hellovulkanwindow-example.html
 *  * https://github.com/bulletphysics/bullet3/blob/master/examples/HelloWorld/HelloWorld.cpp
 */

#include "qttst.h"

#include <QApplication>
#include <QMainWindow>
#include <QLoggingCategory>

#include <cstddef>
#include <cstdint>
#include <locale>
#include <iostream>
#include <random>
#include <ranges>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

#include <boost/scope_exit.hpp>



// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

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



// ----------------------------------------------------------------------------
// 3d object
// ----------------------------------------------------------------------------

/**
 * number of floats in vertex buffer
 */
std::size_t PolyObject::GetNumVertexBufferElements() const
{
	return m_vecVerts.size() + m_vecNorms.size() + m_vecCols.size() + m_vecUVs.size();
}


std::size_t PolyObject::GetNumVertices() const
{
	return m_triangles.size();
}


const t_vec3& PolyObject::GetVertex(std::size_t i) const
{
	return m_triangles[i];
}


const t_vec3& PolyObject::GetUV(std::size_t i) const
{
	return m_triangleuvs[i];
}


static inline btTransform to_bttrafo(const t_mat& _mat)
{
	btMatrix3x3 mat
	{
		_mat(0,0), _mat(0,1), _mat(0,2),
		_mat(1,0), _mat(1,1), _mat(1,2),
		_mat(2,0), _mat(2,1), _mat(2,2)
	};

	btVector3 vec{_mat(0,3), _mat(1,3), _mat(2,3)};

	return btTransform{mat, vec};
}


void PolyObject::CreatePlaneGeometry(
	const t_mat& mat,
	const t_vec3& norm, t_real size,
	t_real r, t_real g, t_real b)
{
	// flatten vertex array into raw float array
	auto to_float_array = [](const std::vector<t_vec3>& verts, int iRepeat=1,
		int iInElems=3, int iOutElems=4, t_real fillElem=1.f) -> std::vector<t_real>
	{
		std::vector<t_real> vecRet;
		vecRet.reserve(iRepeat*verts.size()*iOutElems);

		for(const t_vec3& vert : verts)
		{
			for(int i=0; i<iRepeat; ++i)
				for(int iElem=0; iElem<iOutElems; ++iElem)
					vecRet.push_back(iElem < iInElems ? vert[iElem] : fillElem);
		}
		return vecRet;
	};

	// 3d object
	auto solid = m::create_plane<t_mat, t_vec3>(norm, size);
	std::tie(m_triangles, m_trianglenorms, m_triangleuvs) =
		m::subdivide_triangles<t_vec3>(m::create_triangles<t_vec3>(solid), 2);

	m_vecVerts = to_float_array(m_triangles, 1, 3, 4, 1.f);
	m_vecNorms = to_float_array(m_trianglenorms, 3, 3, 4, 0.f);
	m_vecUVs = to_float_array(m_triangleuvs, 1, 2, 2, 0.f);

	m_vecCols.reserve(4*m_triangles.size());
	for(std::size_t iVert=0; iVert<m_triangles.size(); ++iVert)
	{
		m_vecCols.push_back(r); m_vecCols.push_back(g);
		m_vecCols.push_back(b); m_vecCols.push_back(1);
	}
	m_mat = mat;

	// rigid body
	m_state = std::make_shared<btDefaultMotionState>(to_bttrafo(m_mat));
	m_shape = std::make_shared<btBoxShape>(btVector3{size, 0.01, size});
	m_rigid_body = std::make_shared<btRigidBody>(
		btRigidBody::btRigidBodyConstructionInfo{0, m_state.get(), m_shape.get(), {0, 0, 0}});
}


void PolyObject::CreateCubeGeometry(
	const t_mat& mat,
	t_real size, t_real r, t_real g, t_real b)
{
	// flatten vertex array into raw float array
	auto to_float_array = [](const std::vector<t_vec3>& verts, int iRepeat=1,
		int iInElems=3, int iOutElems=4, t_real fillElem=1.f) -> std::vector<t_real>
	{
		std::vector<t_real> vecRet;
		vecRet.reserve(iRepeat*verts.size()*iOutElems);

		for(const t_vec3& vert : verts)
		{
			for(int i=0; i<iRepeat; ++i)
				for(int iElem=0; iElem<iOutElems; ++iElem)
					vecRet.push_back(iElem < iInElems ? vert[iElem] : fillElem);
		}
		return vecRet;
	};

	// 3d object
	auto solid = m::create_cube<t_vec3>(size);
	std::tie(m_triangles, m_trianglenorms, m_triangleuvs) =
		m::subdivide_triangles<t_vec3>(m::create_triangles<t_vec3>(solid), 2);

	m_vecVerts = to_float_array(m_triangles, 1, 3, 4, 1.f);
	m_vecNorms = to_float_array(m_trianglenorms, 3, 3, 4, 0.f);
	m_vecUVs = to_float_array(m_triangleuvs, 1, 2, 2, 0.f);

	m_vecCols.reserve(4*m_triangles.size());
	for(std::size_t iVert=0; iVert<m_triangles.size(); ++iVert)
	{
		m_vecCols.push_back(r); m_vecCols.push_back(g);
		m_vecCols.push_back(b); m_vecCols.push_back(1);
	}
	m_mat = mat;

	// rigid body
	btScalar mass{1.f};
	btVector3 com{0, 0, 0};
	m_shape = std::make_shared<btBoxShape>(btVector3{size, size, size});
	m_shape->calculateLocalInertia(mass, com);
	m_state = std::make_shared<btDefaultMotionState>(to_bttrafo(m_mat));
	m_rigid_body = std::make_shared<btRigidBody>(
		btRigidBody::btRigidBodyConstructionInfo{mass, m_state.get(), m_shape.get(), com});

}


/**
 * copy vertex info to mapped memory
 */
std::size_t PolyObject::UpdateVertexBuffers(t_real *pMemOrig, std::size_t mem_offs)
{
	// get offset to memory where the current vertex object is to be saved
	t_real *pMem = pMemOrig + mem_offs;
	m_mem_offs = mem_offs;

	std::cout << "Copying " << m_vecVerts.size()/4 << " vertices." << std::endl;
	std::size_t memidx = 0;
	for(std::size_t vertex=0; vertex<m_vecVerts.size()/4; ++vertex)
	{
		// vertex
		pMem[memidx++] = m_vecVerts[vertex*4 + 0];
		pMem[memidx++] = m_vecVerts[vertex*4 + 1];
		pMem[memidx++] = m_vecVerts[vertex*4 + 2];
		pMem[memidx++] = m_vecVerts[vertex*4 + 3];

		// normals
		pMem[memidx++] = m_vecNorms[vertex*4 + 0];
		pMem[memidx++] = m_vecNorms[vertex*4 + 1];
		pMem[memidx++] = m_vecNorms[vertex*4 + 2];
		pMem[memidx++] = m_vecNorms[vertex*4 + 3];

		// colours
		pMem[memidx++] = m_vecCols[vertex*4 + 0];
		pMem[memidx++] = m_vecCols[vertex*4 + 1];
		pMem[memidx++] = m_vecCols[vertex*4 + 2];
		pMem[memidx++] = m_vecCols[vertex*4 + 3];

		// uv coords
		pMem[memidx++] = m_vecUVs[vertex*2 + 0];
		pMem[memidx++] = m_vecUVs[vertex*2 + 1];
	}

	return mem_offs + memidx;
}


std::size_t PolyObject::GetMemOffset() const
{
	return m_mem_offs;
}


void PolyObject::SetMatrix(const t_mat& mat)
{
	m_mat = mat;
}


const t_mat& PolyObject::GetMatrix() const
{
	return m_mat;
}


void PolyObject::tick([[maybe_unused]] const std::chrono::milliseconds& ms)
{
	this->SetMatrixFromState();
}


void PolyObject::SetMatrixFromState()
{
	btTransform trafo{};
	m_rigid_body->getMotionState()->getWorldTransform(trafo);
	btMatrix3x3 mat = trafo.getBasis();
	btVector3 pos = trafo.getOrigin();

	m_mat = m::unit<t_mat>(4);
	for(int row=0; row<3; ++row)
	{
		for(int col=0; col<3; ++col)
			m_mat(row, col) = mat.getRow(row)[col];

		m_mat(row, 3) = pos[row];
	}
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// vk renderer
// ----------------------------------------------------------------------------

VkRenderer::VkRenderer(std::shared_ptr<QVulkanInstance>& vk,
	std::shared_ptr<btDynamicsWorld> world, VkWnd* wnd)
	: m_world{world}, m_vkinst{vk}, m_vkwnd{wnd}
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;

	// create some objects
	{
		// plane
		PolyObject plane;
		plane.CreatePlaneGeometry(
			m::hom_translation<t_mat, t_real>(0, -1, 0),
			m::create<t_vec3>({0, 1, 0}), 5., 0., 0., 1.);
		m_world->addRigidBody(plane.GetRigidBody().get());
		m_objs.emplace_back(std::move(plane));

		// box 1
		PolyObject box;
		box.CreateCubeGeometry(
			m::hom_translation<t_mat, t_real>(0, 10, 0) *
				m::rotation<t_mat, t_vec>(m::create<t_vec>({1,0,0}), m::pi<t_real>*0.25) *
				m::rotation<t_mat, t_vec>(m::create<t_vec>({0,1,0}), m::pi<t_real>*0.25),
			1., 1., 0., 0.);
		m_world->addRigidBody(box.GetRigidBody().get());
		m_objs.emplace_back(std::move(box));

		// box 2
		PolyObject box2;
		box2.CreateCubeGeometry(
			m::hom_translation<t_mat, t_real>(0, 15, 0.25),
			1., 1., 0., 0.);
		m_world->addRigidBody(box2.GetRigidBody().get());
		m_objs.emplace_back(std::move(box2));

	}

	m_cam.SetTranslation(0., 0., -3.);
	m_cam.Update();
}


VkRenderer::~VkRenderer()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}


QPointF VkRenderer::VkToScreenCoords(const t_vec& vec4, bool *pVisible)
{
	auto [ vecPersp, vec ] =
		m::hom_to_screen_coords<t_mat, t_vec>(vec4,
			m_cam.GetMatrix(), m_matPerspective, m_matViewport, true);

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
	const t_real moveDelta = 0.015 * ms.count();
	const t_real rotateDelta = 0.001 * ms.count() * m::pi<t_real>;

	m_cam.Translate(0, m_moving[0]*moveDelta);
	m_cam.Translate(1, m_moving[1]*moveDelta);
	m_cam.Translate(2, m_moving[2]*moveDelta);

	m_cam.Rotate(0, m_rotating[0]*rotateDelta);
	m_cam.Rotate(1, m_rotating[1]*rotateDelta);
	m_cam.Rotate(2, m_rotating[2]*rotateDelta);

	m_cam.Update();

	for(auto& obj : m_objs)
		obj.tick(ms);

	UpdatePicker();

	if(m_vkwnd)
		m_vkwnd->requestUpdate();
}


void VkRenderer::SetMousePos(const QPointF& pt)
{
	m_posMouse = pt;
	UpdatePicker();
}


t_vec3 hom_trafo(const t_mat& mat, const t_vec3& vec3, bool is_pos=1)
{
	t_vec vec4 = m::create<t_vec>({ vec3[0], vec3[1], vec3[2], is_pos ? 1.f : 0.f });
	vec4 = mat * vec4;

	return m::create<t_vec3>({ vec4[0], vec4[1], vec4[2] });
}


void VkRenderer::UpdatePicker()
{
	auto [org, dir] = m::hom_line_from_screen_coords<t_mat, t_vec>(
		m_posMouse.x(), m_posMouse.y(), 0., 1.,
		m_cam.GetMatrixInv(), m_matPerspective_inv,
		m_matViewport_inv, &m_matViewport, false);

	const auto& obj = m_objs[0];	// only intersect with first object
	const auto& matObj = obj.GetMatrix();

	for(std::size_t startidx=0; startidx+2<obj.GetNumVertices(); startidx+=3)
	{
		std::vector<t_vec3> poly
		{{
			hom_trafo(matObj, obj.GetVertex(startidx + 0), 1),
			hom_trafo(matObj, obj.GetVertex(startidx + 1), 1),
			hom_trafo(matObj, obj.GetVertex(startidx + 2), 1),
		}};

		auto [vecInters, bInters, lamInters] =
			m::intersect_line_poly<t_vec3>(
				t_vec3(org[0], org[1], org[2]),
				t_vec3(dir[0], dir[1], dir[2]),
				poly);

		if(bInters)
		{
			std::vector<t_vec3> polyuv
			{{
				obj.GetUV(startidx + 0),
				obj.GetUV(startidx + 1),
				obj.GetUV(startidx + 2)
			}};

			using t_mat_tmp = m::mat<t_real>;
			auto uv = m::poly_uv<t_mat_tmp, t_vec3>(poly[0], poly[1], poly[2],
				polyuv[0], polyuv[1], polyuv[2], vecInters);

			m_veccurUV[0] = uv[0]; m_veccurUV[1] = uv[1];
		}
	}
}


void VkRenderer::preInitResources()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}


/**
 * shaders
 * https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#shaders
 */
std::vector<VkPipelineShaderStageCreateInfo> VkRenderer::CreateShaders()
{
	if(!fs::exists("vert.spv") || !fs::exists("frag.spv"))
	{
		std::cerr << "Vertex or fragment shader could not be found." << std::endl;
		return std::vector<VkPipelineShaderStageCreateInfo>{};
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
		if(std::ifstream ifstr{file}; !ifstr.read(reinterpret_cast<char*>(std::ranges::data(bin)), size))
		{
			std::cerr << "Error loading shader " << file << "." << std::endl;
			continue;
		}

		// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkShaderModuleCreateInfo.html
		VkShaderModuleCreateInfo shaderInfo
		{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // fixed value
			.pNext = nullptr,
			.flags = 0, // unused
			.codeSize = size,
			.pCode = reinterpret_cast<decltype(VkShaderModuleCreateInfo::pCode)>(std::ranges::cdata(bin))
		};

		// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateShaderModule.html
		if(VkResult err = m_vkfuncs->vkCreateShaderModule(m_vkdev, &shaderInfo, 0, std::get<1>(shader));
		   err != VK_SUCCESS)
		{
			std::cerr << "Error compiling " << file <<  ": " << get_vk_error(err) << std::endl;
			continue;
		}
	}

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineShaderStageCreateInfo.html
	std::vector<VkPipelineShaderStageCreateInfo> shaderstages
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

	return shaderstages;
}


std::size_t VkRenderer::GetNumShaderInputElements() const
{
	return 3*4	// vec4 vertex, normal, vertexcolor
		+ 2;	// vec2 texcoords
}


/**
 * memory size of the uniform buffer
 */
std::size_t VkRenderer::GetUniformBufferSize(bool use_granularity) const
{
	const auto& matCam = m_cam.GetMatrix();

	std::size_t size = (m_matPerspective.size1() * m_matPerspective.size2()) * sizeof(t_real);
	size += (matCam.size1() * matCam.size2()) * sizeof(t_real);
	size += (4*4) * sizeof(t_real);	// object matrix
	size += (m_veccurUV.size()) * sizeof(t_real);
	size += sizeof(std::int32_t);	// cursor active flag

	if(use_granularity)
		size = m::next_multiple(size, m_bufferoffsetgranularity);

	return size;
}


/**
 * size of vertex buffer
 */
std::size_t VkRenderer::GetFullSizeVertexBuffer(bool use_granularity) const
{
	std::size_t size = 0;
	for(const auto& obj : m_objs)
		size += obj.GetNumVertexBufferElements()*sizeof(t_real);

	if(use_granularity)
		size = m::next_multiple(size, m_bufferoffsetgranularity);
	return size;
}


/**
 * size of buffer
 */
std::size_t VkRenderer::GetFullSizeUniformBuffer(bool use_granularity) const
{
	std::size_t size = GetUniformBufferSize(use_granularity);

	// create a copy of the uniforms for each object
	size *= m_objs.size();

	return size;
}


/**
 * create vertex and uniform buffers
 */
void VkRenderer::CreateBuffers()
{
	if(!m_vkfuncs)
		return;

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkBufferCreateInfo.html
	std::uint32_t queryfamilyindices[] = {};

	VkBufferCreateInfo buffercreateinfo
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.size = GetFullSizeVertexBuffer(true) + m_vkwnd->concurrentFrameCount()*GetFullSizeUniformBuffer(true),
		.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE /*VK_SHARING_MODE_CONCURRENT*/,
		.queueFamilyIndexCount = sizeof(queryfamilyindices) / sizeof(queryfamilyindices[0]),
		.pQueueFamilyIndices = queryfamilyindices,
	};

	if(VkResult err = m_vkfuncs->vkCreateBuffer(m_vkdev, &buffercreateinfo, 0, &m_buffer);
	   err != VK_SUCCESS)
	{
		std::cerr << "Error creating buffer: " << get_vk_error(err) << std::endl;
		return;
	}

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryRequirements.html
	VkMemoryRequirements buffer_requirements{};
	m_vkfuncs->vkGetBufferMemoryRequirements(m_vkdev, m_buffer, &buffer_requirements);
	std::cout << "Buffer requested size: " << buffercreateinfo.size
		<< ", required size: " << buffer_requirements.size
		<< ", required alignment: " << buffer_requirements.alignment	// == m_bufferoffsetgranularity
		<< "." << std::endl;

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryAllocateInfo.html
	VkMemoryAllocateInfo memallocinfo
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = nullptr,
		.allocationSize = buffer_requirements.size,
		.memoryTypeIndex = m_vkwnd->hostVisibleMemoryIndex(),
	};

	if(VkResult err = m_vkfuncs->vkAllocateMemory(m_vkdev, &memallocinfo, 0, &m_mem);
	   err != VK_SUCCESS)
	{
		std::cerr << "Error allocating memory: " << get_vk_error(err) << std::endl;
		return;
	}

	if(VkResult err = m_vkfuncs->vkBindBufferMemory(m_vkdev, m_buffer, m_mem, 0);
	   err != VK_SUCCESS)
	{
		std::cerr << "Error binding memory to buffer: " << get_vk_error(err) << std::endl;
		return;
	}
}


void VkRenderer::CreatePipelineLayout()
{
	if(!m_vkfuncs)
		return;

	VkPushConstantRange pushconstranges[] = {};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineLayoutCreateInfo.html
	VkPipelineLayoutCreateInfo layoutcreateinfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = sizeof(m_setlayouts)/sizeof(m_setlayouts[0]),
		.pSetLayouts = m_setlayouts,
		.pushConstantRangeCount = sizeof(pushconstranges) / sizeof(pushconstranges[0]),
		.pPushConstantRanges = pushconstranges,
	};

	if(VkResult err = m_vkfuncs->vkCreatePipelineLayout(m_vkdev, &layoutcreateinfo, 0, &m_layout);
		err != VK_SUCCESS)
	{
		std::cerr << "Error creating graphics pipeline layout: " << get_vk_error(err) << std::endl;
		return;
	}
}



void VkRenderer::CreatePipelineCache()
{
	if(!m_vkfuncs)
		return;

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
}


/**
 * create various pipeline stages
 */
std::tuple<
	VkPipelineInputAssemblyStateCreateInfo,
	VkPipelineTessellationStateCreateInfo,
	VkPipelineViewportStateCreateInfo,
	VkPipelineRasterizationStateCreateInfo,
	VkPipelineMultisampleStateCreateInfo,
	VkPipelineDepthStencilStateCreateInfo>
VkRenderer::CreatePipelineStages() const
{
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
		.viewportCount = sizeof(m_viewports) / sizeof(m_viewports[0]),
		.pViewports = m_viewports,	/* nullptr => dynamically set */
		.scissorCount = sizeof(m_viewrects) / sizeof(m_viewrects[0]),
		.pScissors = m_viewrects,	/* nullptr => dynamically set */
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
		.cullMode = /*VK_CULL_MODE_BACK_BIT*/ VK_CULL_MODE_NONE,
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

	return std::make_tuple(inputassemblystate, tessellationstate,
		viewportstate, rasterisationstate,
		multisamplestate, depthstencilstate);
}


void VkRenderer::initResources()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;

	m_vkdev = m_vkwnd->device();
	m_vkfuncs = m_vkinst->deviceFunctions(m_vkdev);

	const VkPhysicalDeviceProperties* props = m_vkwnd->physicalDeviceProperties();
	const VkPhysicalDeviceLimits& limits = props->limits;
	m_bufferoffsetgranularity = limits.minUniformBufferOffsetAlignment;

	std::cout << "Physical device:"
		<< "\n\tapi = " << props->apiVersion << ","
		<< "\n\tdriver = " << props->driverVersion << ","
		<< "\n\tvendor = " << props->vendorID << ","
		<< "\n\tdevice = " << props->deviceID << ","
		<< "\n\tname = " << props->deviceName << ","
		<< "\n\ttype = " << get_device_type(props->deviceType) << ","
		<< "\n\tminimum uniform buffer offset alignment = " << m_bufferoffsetgranularity << "."
		<< std::endl;

	std::cout << "Concurrent frame count: " << m_vkwnd->concurrentFrameCount() << std::endl;


	std::vector<VkPipelineShaderStageCreateInfo> shaderstages = CreateShaders();
	CreateBuffers();


	// --------------------------------------------------------------------
	// shader input descriptions
	// --------------------------------------------------------------------
	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkVertexInputBindingDescription.html
	VkVertexInputBindingDescription vertinputbindingdesc[]
	{
		{
			.binding = 0,	// corresponds to the binding in the shader
			.stride = std::uint32_t(GetNumShaderInputElements() * sizeof(t_real)),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkVertexInputAttributeDescription.html
	VkVertexInputAttributeDescription vertinputattrdesc[]
	{
		{
			.location = 0,	// vertex
			.binding = vertinputbindingdesc[0].binding,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = 0,
		},
		{
			.location = 1,	// normal
			.binding = vertinputbindingdesc[0].binding,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = 1*4*sizeof(t_real),
		},
		{
			.location = 2,	// colour
			.binding = vertinputbindingdesc[0].binding,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = 2*4*sizeof(t_real),
		},
		{
			.location = 3,	// uv coords
			.binding = vertinputbindingdesc[0].binding,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = 3*4*sizeof(t_real),
		},
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineVertexInputStateCreateInfo.html
	VkPipelineVertexInputStateCreateInfo vertexinputstate
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.vertexBindingDescriptionCount = sizeof(vertinputbindingdesc)/sizeof(vertinputbindingdesc[0]),
		.pVertexBindingDescriptions = vertinputbindingdesc,
		.vertexAttributeDescriptionCount = sizeof(vertinputattrdesc)/sizeof(vertinputattrdesc[0]),
		.pVertexAttributeDescriptions = vertinputattrdesc,
	};
	// --------------------------------------------------------------------


	// --------------------------------------------------------------------
	// shader uniform descriptions
	// --------------------------------------------------------------------
	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkDescriptorSetLayoutBinding.html
	VkDescriptorSetLayoutBinding setlayoutbindings[]
	{
		{
			.binding = vertinputbindingdesc[0].binding,
			// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkDescriptorType.html
			.descriptorType = /*VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER*/ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			.descriptorCount = 1,
			// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkShaderStageFlagBits.html
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT /*| VK_SHADER_STAGE_FRAGMENT_BIT*/,
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


	VkDescriptorPoolSize poolSizes[]
	{
		{
			.type = setlayoutbindings[0].descriptorType,
			.descriptorCount = (std::uint32_t)m_vkwnd->concurrentFrameCount()
		},
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkDescriptorPoolCreateInfo.html
	VkDescriptorPoolCreateInfo poolcreateinfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.maxSets = poolSizes[0].descriptorCount,
		.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]),
		.pPoolSizes = poolSizes,
	};

	if(VkResult err = m_vkfuncs->vkCreateDescriptorPool(m_vkdev, &poolcreateinfo, 0, &m_descrpool);
	   err != VK_SUCCESS)
	{
		std::cerr << "Error creating descriptor pool: " << get_vk_error(err) << std::endl;
		return;
	}


	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkDescriptorSetAllocateInfo.html
	VkDescriptorSetAllocateInfo allocinfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = m_descrpool,
		.descriptorSetCount = sizeof(m_setlayouts) / sizeof(m_setlayouts[0]),
		.pSetLayouts = m_setlayouts,
	};

	m_descrset = std::shared_ptr<VkDescriptorSet[]>
		{ new VkDescriptorSet[poolSizes[0].descriptorCount] };
	m_descrbufferinfo = std::shared_ptr<VkDescriptorBufferInfo[]>
		{ new VkDescriptorBufferInfo[poolSizes[0].descriptorCount] };

	// each frame has a copy of all uniforms
	for(std::uint32_t frame=0; frame<poolSizes[0].descriptorCount; ++frame)
	{
		if(VkResult err = m_vkfuncs->vkAllocateDescriptorSets(m_vkdev, &allocinfo, &m_descrset[frame]);
		   err != VK_SUCCESS)
		{
			std::cerr << "Error allocating descriptor sets: " << get_vk_error(err) << std::endl;
			return;
		}

		m_descrbufferinfo[frame].buffer = m_buffer;
		m_descrbufferinfo[frame].range = GetFullSizeUniformBuffer(true);

		if(frame == 0)
		{
			m_descrbufferinfo[frame].offset = GetFullSizeVertexBuffer(true);
		}
		else
		{
			m_descrbufferinfo[frame].offset =
				m_descrbufferinfo[frame-1].offset + m_descrbufferinfo[frame-1].range;
		}

		// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkWriteDescriptorSet.html
		VkWriteDescriptorSet writedescrset[]
		{
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = m_descrset[frame],
				.dstBinding = setlayoutbindings[0].binding,
				.dstArrayElement = 0,
				.descriptorCount = sizeof(setlayoutbindings) / sizeof(setlayoutbindings[0]),
				.descriptorType = setlayoutbindings[0].descriptorType,
				.pImageInfo = nullptr,						//
				.pBufferInfo = &m_descrbufferinfo[frame],	// two of these have to be 0
				.pTexelBufferView = nullptr,				//
			},
		};

		// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkCopyDescriptorSet.html
		VkCopyDescriptorSet copydescrset[]
		{};

		m_vkfuncs->vkUpdateDescriptorSets(m_vkdev,
			sizeof(writedescrset)/sizeof(writedescrset[0]), writedescrset,
			sizeof(copydescrset)/sizeof(copydescrset[0]), copydescrset);
	}
	// --------------------------------------------------------------------



	// --------------------------------------------------------------------
	// pipeline stages
	// --------------------------------------------------------------------
	CreatePipelineLayout();
	CreatePipelineCache();

	auto [inputassemblystate, tessellationstate,
		viewportstate, rasterisationstate,
		multisamplestate, depthstencilstate] = CreatePipelineStages();


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


	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkGraphicsPipelineCreateInfo.html
	VkGraphicsPipelineCreateInfo createInfos[]
	{
		{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stageCount = (std::uint32_t)shaderstages.size(),
			.pStages = &shaderstages[0],
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
		},
	};

	if(VkResult err = m_vkfuncs->vkCreateGraphicsPipelines(m_vkdev, m_cache,
		sizeof(createInfos)/sizeof(createInfos[0]), createInfos, nullptr, &m_pipeline);
		err != VK_SUCCESS)
	{
		std::cerr << "Error creating graphics pipeline: " << get_vk_error(err) << std::endl;
	}
	// --------------------------------------------------------------------


	UpdateVertexBuffers();
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

		// buffer
		reinterpret_cast<vkhandle>(m_mem),
		reinterpret_cast<vkhandle>(m_buffer),

		// set layouts
		reinterpret_cast<vkhandle>(m_setlayouts[0]),
		reinterpret_cast<vkhandle>(m_descrpool),

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

		// buffer
		reinterpret_cast<t_destroyfunc>(&QVulkanDeviceFunctions::vkFreeMemory),
		reinterpret_cast<t_destroyfunc>(&QVulkanDeviceFunctions::vkDestroyBuffer),

		// set layouts
		reinterpret_cast<t_destroyfunc>(&QVulkanDeviceFunctions::vkDestroyDescriptorSetLayout),
		reinterpret_cast<t_destroyfunc>(&QVulkanDeviceFunctions::vkDestroyDescriptorPool),

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


void VkRenderer::TogglePerspective()
{
	m_use_prespective_proj = !m_use_prespective_proj;
	UpdatePerspective();
}


void VkRenderer::UpdatePerspective()
{
	using namespace m_ops;

	// perspective projection
	if(m_use_prespective_proj)
	{
		m_matPerspective = m::hom_perspective<t_mat>(
			0.01, 100., m::pi<t_real>*0.5,
			t_real(m_iScreenDims[1])/t_real(m_iScreenDims[0]), false, true, true);
	}

	// orthogonal projection
	else
	{
		m_matPerspective = m::hom_parallel<t_mat>(
			0.01, 100., -4., 4., -4, 4., false, true, true);
	}

	std::tie(m_matPerspective_inv, std::ignore) = m::inv<t_mat, t_vec>(m_matPerspective);
	std::cout << "projection matrix: " << m_matPerspective << "." << std::endl;
	std::cout << "inverted projection matrix: " << m_matPerspective_inv << "." << std::endl;
}


void VkRenderer::initSwapChainResources()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;

	m_iScreenDims[0] = m_vkwnd->swapChainImageSize().width();
	m_iScreenDims[1] = m_vkwnd->swapChainImageSize().height();
	std::cout << "window size: " << m_iScreenDims[0] << " x " << m_iScreenDims[1] << "." << std::endl;

	// viewport
	m_matViewport = m::hom_viewport<t_mat>(m_iScreenDims[0], m_iScreenDims[1], 0., 1.);
	std::tie(m_matViewport_inv, std::ignore) = m::inv<t_mat, t_vec>(m_matViewport);

	m_viewports[0] = VkViewport
	{
		.x = 0, .y = 0,
		.width = (t_real)m_iScreenDims[0], .height = (t_real)m_iScreenDims[1],
		.minDepth = 0, .maxDepth = 1,
	};

	m_viewrects[0] = VkRect2D
	{
		.offset = VkOffset2D { .x = 0, .y = 0 },
		.extent = VkExtent2D { .width = m_iScreenDims[0], .height = m_iScreenDims[1] },
	};

	UpdatePerspective();
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


/**
 * copy vertex info to mapped memory
 */
void VkRenderer::UpdateVertexBuffers()
{
	if(!m_vkfuncs)
		return;

	BOOST_SCOPE_EXIT(&m_vkfuncs, &m_vkdev, &m_mem)
	{
		m_vkfuncs->vkUnmapMemory(m_vkdev, m_mem);
	}
	BOOST_SCOPE_EXIT_END

	t_real* pMem = nullptr;
	if(VkResult err = m_vkfuncs->vkMapMemory(m_vkdev, m_mem, 0, VK_WHOLE_SIZE, 0, (void**)&pMem);
	   err != VK_SUCCESS)
	{
		std::cerr << "Error mapping vertex memory: " << get_vk_error(err) << std::endl;
		return;
	}

	std::size_t mem_offs = 0;
	for(auto& obj : m_objs)
	{
		mem_offs = obj.UpdateVertexBuffers(pMem, mem_offs);
	}
}


/**
 * copy uniforms to mapped memory
 */
void VkRenderer::UpdateUniforms()
{
	if(!m_vkfuncs)
		return;

	BOOST_SCOPE_EXIT(&m_vkfuncs, &m_vkdev, &m_mem)
	{
		m_vkfuncs->vkUnmapMemory(m_vkdev, m_mem);
	}
	BOOST_SCOPE_EXIT_END

	t_real* pMem = nullptr;
	if(VkResult err = m_vkfuncs->vkMapMemory(m_vkdev, m_mem,
		m_descrbufferinfo[m_vkwnd->currentFrame()].offset,
		m_descrbufferinfo[m_vkwnd->currentFrame()].range, 0, (void**)&pMem);
	err != VK_SUCCESS)
	{
		std::cerr << "Error mapping uniform memory: " << get_vk_error(err) << std::endl;
		return;
	}

	const auto& matCam = m_cam.GetMatrix();

	// create a copy of the uniforms for each object
	for(std::size_t objidx=0; objidx<m_objs.size(); ++objidx)
	{
		const auto& obj = m_objs[objidx];
		const t_mat& matObj = obj.GetMatrix();

		t_real* pCurMem = reinterpret_cast<t_real*>(
			reinterpret_cast<std::int8_t*>(pMem) + GetUniformBufferSize(true)*objidx);

		const std::size_t persp_start_idx = 0;
		const std::size_t cam_start_idx = 1*4*4;
		const std::size_t obj_start_idx = 2*4*4;
		const std::size_t cursor_start_idx = 3*4*4;
		const std::size_t cursor_active_start_idx = 3*4*4 + 2;

		// matrices
		for(std::size_t i=0; i<4; ++i)
		{
			for(std::size_t j=0; j<4; ++j)
			{
				// perspective matrix
				pCurMem[persp_start_idx + j*4 + i] = m_matPerspective(i,j);

				// camera matrix
				pCurMem[cam_start_idx + j*4 + i] = matCam(i,j);

				// object matrix
				pCurMem[obj_start_idx + j*4 + i] = matObj(i,j);
			}
		}

		// current cursor coords
		pCurMem[cursor_start_idx + 0] = m_veccurUV[0];
		pCurMem[cursor_start_idx + 1] = m_veccurUV[1];

		// cursor active flag
		pCurMem[cursor_active_start_idx] = (objidx == 0);
	}
}


void VkRenderer::startNextFrame()
{
	UpdateUniforms();

	VkClearValue clr[] /* union between .color and .depthStencil */
	{
		{ .color = VkClearColorValue{ .float32 = {1.f, 1.f, 1.f, 1.f} } },
		{ .depthStencil = VkClearDepthStencilValue{ .depth = 1.f, .stencil = 0 } },
	};

	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkRenderPassBeginInfo.html
	VkRenderPassBeginInfo beg
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,	// fixed value
		.pNext = nullptr,
		.renderPass = m_vkwnd->defaultRenderPass(),
		.framebuffer = m_vkwnd->currentFramebuffer(),
		.renderArea = VkRect2D
		{
			.offset = VkOffset2D { .x = 0, .y = 0 },
			.extent = VkExtent2D { .width = m_iScreenDims[0], .height = m_iScreenDims[1] },
		},
		.clearValueCount = sizeof(clr) / sizeof(clr[0]),
		.pClearValues = clr,
	};

	VkSubpassContents cont = VK_SUBPASS_CONTENTS_INLINE /*VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS*/;
	VkPipelineBindPoint bindpoint = VK_PIPELINE_BIND_POINT_GRAPHICS /*VK_PIPELINE_BIND_POINT_COMPUTE*/;

	BOOST_SCOPE_EXIT(&m_vkfuncs, &m_vkwnd)
	{
		m_vkfuncs->vkCmdEndRenderPass(m_vkwnd->currentCommandBuffer());
		m_vkwnd->frameReady();
	}
	BOOST_SCOPE_EXIT_END
	m_vkfuncs->vkCmdBeginRenderPass(m_vkwnd->currentCommandBuffer(), &beg, cont);

	m_vkfuncs->vkCmdSetViewport(m_vkwnd->currentCommandBuffer(), 0,
		sizeof(m_viewports)/sizeof(m_viewports[0]), m_viewports);
	m_vkfuncs->vkCmdSetScissor(m_vkwnd->currentCommandBuffer(), 0,
		sizeof(m_viewrects)/sizeof(m_viewrects[0]), m_viewrects);

	m_vkfuncs->vkCmdBindPipeline(m_vkwnd->currentCommandBuffer(), bindpoint, m_pipeline);

	for(std::size_t i=0; i<m_objs.size(); ++i)
	{
		const auto& obj = m_objs[i];

		// offsets into uniform buffer for each draw command (for VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		std::uint32_t dynOffs[] = { std::uint32_t(i*GetUniformBufferSize(true)) };
		std::uint32_t numDescrSets = 1;
		m_vkfuncs->vkCmdBindDescriptorSets(m_vkwnd->currentCommandBuffer(), bindpoint, m_layout,
			0, numDescrSets, &m_descrset[m_vkwnd->currentFrame()],
			sizeof(dynOffs)/sizeof(dynOffs[0]), dynOffs);

		// offsets into vertex buffer
		VkDeviceSize bufferoffs[] = { obj.GetMemOffset()*sizeof(t_real) };
		std::uint32_t startVertex = 0;
		std::uint64_t numVertices = obj.GetNumVertices();
		std::uint32_t numBindings = 1;

		m_vkfuncs->vkCmdBindVertexBuffers(m_vkwnd->currentCommandBuffer(), 0,
			numBindings, &m_buffer, bufferoffs);

		m_vkfuncs->vkCmdDraw(m_vkwnd->currentCommandBuffer(), numVertices, 1, startVertex, 0);
	}
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// vk window
// ----------------------------------------------------------------------------

VkWnd::VkWnd(std::shared_ptr<QVulkanInstance>& vk,
	std::shared_ptr<btDynamicsWorld> world, QWindow* parent)
	: QVulkanWindow{parent}, m_vkinst{vk}, m_world{world}
{
	setVulkanInstance(m_vkinst.get());

	std::chrono::milliseconds ticks{1000 / 60};
	connect(&m_timer, &QTimer::timeout,
		[this, ticks]() -> void
		{
			if(!m_vkrenderer)
				return;

			m_world->stepSimulation(t_real(ticks.count()) / 1000.);
			m_vkrenderer->tick(ticks);
			m_runningtime += ticks;

			t_vec pos = m_vkrenderer->GetCamera().GetPosition();

			auto status = QString{"Running time: %1 s, camera: %2, %3, %4"}
				.arg(m_runningtime.count()/1000, 0)
				.arg(pos[0], 0, 'f', 1)
				.arg(pos[1], 0, 'f', 1)
				.arg(pos[2], 0, 'f', 1);
			emit EmitStatusMsg(status);
		});

	m_timer.start(ticks);
}


VkWnd::~VkWnd()
{
	m_timer.stop();
}


QVulkanWindowRenderer* VkWnd::createRenderer()
{
	if(m_vkrenderer)
		delete m_vkrenderer;

	return m_vkrenderer = new VkRenderer(m_vkinst, m_world, this);
}


void VkWnd::mouseMoveEvent(QMouseEvent *pEvt)
{
	if(m_vkrenderer)
		m_vkrenderer->SetMousePos(pEvt->localPos());

	QVulkanWindow::mouseMoveEvent(pEvt);
}


void VkWnd::keyPressEvent(QKeyEvent *pEvt)
{
	if(!m_vkrenderer)
		return;

	if(pEvt->key() == Qt::Key_Space)
		m_vkrenderer->TogglePerspective();

	if(pEvt->key() == Qt::Key_A)
		m_vkrenderer->SetMoving(0, 1.);
	if(pEvt->key() == Qt::Key_D)
		m_vkrenderer->SetMoving(0, -1.);
	if(pEvt->key() == Qt::Key_W)
		m_vkrenderer->SetMoving(2, 1.);
	if(pEvt->key() == Qt::Key_S)
		m_vkrenderer->SetMoving(2, -1.);
	if(pEvt->key() == Qt::Key_E)
		m_vkrenderer->SetMoving(1, 1.);
	if(pEvt->key() == Qt::Key_Q)
		m_vkrenderer->SetMoving(1, -1.);

	if(pEvt->key() == Qt::Key_Up)
		m_vkrenderer->SetRotating(0, 1.);
	if(pEvt->key() == Qt::Key_Down)
		m_vkrenderer->SetRotating(0, -1.);
	if(pEvt->key() == Qt::Key_Left)
		m_vkrenderer->SetRotating(1, -1.);
	if(pEvt->key() == Qt::Key_Right)
		m_vkrenderer->SetRotating(1, 1.);
	if(pEvt->key() == Qt::Key_Y)
		m_vkrenderer->SetRotating(2, -1.);
	if(pEvt->key() == Qt::Key_C)
		m_vkrenderer->SetRotating(2, 1.);

	QVulkanWindow::keyPressEvent(pEvt);
}


void VkWnd::keyReleaseEvent(QKeyEvent *pEvt)
{
	if(!m_vkrenderer)
		return;

	if(pEvt->key() == Qt::Key_A || pEvt->key() == Qt::Key_D)
		m_vkrenderer->SetMoving(0, 0.);
	if(pEvt->key() == Qt::Key_W || pEvt->key() == Qt::Key_S)
		m_vkrenderer->SetMoving(2, 0.);
	if(pEvt->key() == Qt::Key_E || pEvt->key() == Qt::Key_Q)
		m_vkrenderer->SetMoving(1, 0.);

	if(pEvt->key() == Qt::Key_Up || pEvt->key() == Qt::Key_Down)
		m_vkrenderer->SetRotating(0, 0.);
	if(pEvt->key() == Qt::Key_Left || pEvt->key() == Qt::Key_Right)
		m_vkrenderer->SetRotating(1, 0.);
	if(pEvt->key() == Qt::Key_Y || pEvt->key() == Qt::Key_C)
		m_vkrenderer->SetRotating(2, 0.);

	QVulkanWindow::keyReleaseEvent(pEvt);
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// main window
// ----------------------------------------------------------------------------
Wnd::Wnd(VkWnd *vkwnd, QWidget* parent) : QMainWindow(parent), m_vkwnd{vkwnd}
{
	// set the vk window as central widget
	m_vkwidget = QWidget::createWindowContainer(m_vkwnd);
	m_vkwidget->setFocusPolicy(Qt::StrongFocus);
	//setFocusProxy(m_vkwidget);
	setCentralWidget(m_vkwidget);

	m_statusbar = new QStatusBar(this);
	m_statuslabel = new QLabel(m_statusbar);
	m_statusbar->addPermanentWidget(m_statuslabel, 0);
	setStatusBar(m_statusbar);

	connect(m_vkwnd, &VkWnd::EmitStatusMsg,
		[this](const QString& str)->void
		{
			if(!m_statuslabel)
				return;

			m_statuslabel->setText(str);
		});
}


Wnd::~Wnd()
{
}


void Wnd::resizeEvent(QResizeEvent *evt)
{
	QMainWindow::resizeEvent(evt);
	//m_vkwidget->setFocus();
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
	// ------------------------------------------------------------------------
	// misc initialisation
	// ------------------------------------------------------------------------
	QLoggingCategory::setFilterRules("*=true\n*.debug=false\n");
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
	// ------------------------------------------------------------------------



	// ------------------------------------------------------------------------
	// bullet
	// ------------------------------------------------------------------------
	auto coll = std::make_shared<btDefaultCollisionConfiguration>(btDefaultCollisionConstructionInfo{});
	auto disp = std::make_shared<btCollisionDispatcherMt>(coll.get());
	auto cache = std::make_shared<btDbvtBroadphase>();
	auto solver = std::make_shared<btSequentialImpulseConstraintSolver>();
	auto world = std::make_shared<btDiscreteDynamicsWorld>(disp.get(), cache.get(), solver.get(), coll.get());

	world->setGravity({0, -9.81, 0});
	// ------------------------------------------------------------------------



	// ------------------------------------------------------------------------
	// vk
	// ------------------------------------------------------------------------
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

	BOOST_SCOPE_EXIT(&vk)
	{
		vk->destroy();
	}
	BOOST_SCOPE_EXIT_END


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

	// create main and vk window
	auto vkwnd = new VkWnd(vk, world);
	auto wnd = std::make_unique<Wnd>(vkwnd);
	wnd->resize(800, 600);
	wnd->show();
	// ------------------------------------------------------------------------


	// run application
	return app->exec();
}
// ----------------------------------------------------------------------------
