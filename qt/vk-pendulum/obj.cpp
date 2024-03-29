/**
 * 3d object
 * @author Tobias Weber
 * @date Feb-2021
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *  * https://code.qt.io/cgit/qt/qtbase.git/tree/examples/vulkan/shared/trianglerenderer.cpp
 *  * https://doc.qt.io/qt-5/qvulkanwindow.html
 *  * https://doc.qt.io/qt-5/qvulkaninstance.html
 *  * https://doc.qt.io/qt-5/qvulkanwindowrenderer.html
 *  * https://doc.qt.io/qt-5/qtgui-hellovulkanwindow-example.html
 *  * https://github.com/bulletphysics/bullet3/blob/master/examples/HelloWorld/HelloWorld.cpp
 */

#include "obj.h"

#include <iostream>


/**
 * number of floats in vertex buffer
 */
std::size_t PolyObject::GetNumVertexBufferElements() const
{
	return m_vecVerts.size() +
		m_vecNorms.size() +
		m_vecCols.size() +
		m_vecUVs.size();
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


/**
 * flatten vertex array into raw float array
 */
static std::vector<t_real> to_float_array(
	const std::vector<t_vec3>& verts, int iRepeat=1,
	int iInElems=3, int iOutElems=4, t_real fillElem=1.f)
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


void PolyObject::CreatePlaneGeometry(
	const t_mat& mat,
	const t_vec3& norm, t_real size,
	t_real r, t_real g, t_real b)
{
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
}


void PolyObject::CreateCubeGeometry(
	const t_mat& mat,
	t_real size,
	t_real r, t_real g, t_real b)
{
	// 3d object
	auto solid = m::create_cube<t_vec3>(size);
	std::tie(m_triangles, m_trianglenorms, m_triangleuvs) =
		m::subdivide_triangles<t_vec3>(
			m::create_triangles<t_vec3>(solid), 2);

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
}


void PolyObject::CreateSphereGeometry(
	const t_mat& mat,
	t_real rad,
	t_real r, t_real g, t_real b)
{
	// 3d object
	auto solid = m::create_icosahedron<t_vec3>(1);
	std::tie(m_triangles, m_trianglenorms, m_triangleuvs) =
		m::spherify<t_vec3>(
			m::subdivide_triangles<t_vec3>(
				m::create_triangles<t_vec3>(solid), 2), rad);

	m_vecVerts = to_float_array(m_triangles, 1, 3, 4, 1.f);
	m_vecNorms = to_float_array(m_trianglenorms, 3, 3, 4, 0.f);
	m_vecUVs = to_float_array(m_triangleuvs, 1, 2, 2, 0.f);
	// TODO
	if(!m_vecUVs.size())
		m_vecUVs.resize(m_triangles.size()*2);

	m_vecCols.reserve(4*m_triangles.size());
	for(std::size_t iVert=0; iVert<m_triangles.size(); ++iVert)
	{
		m_vecCols.push_back(r); m_vecCols.push_back(g);
		m_vecCols.push_back(b); m_vecCols.push_back(1);
	}

	m_mat = mat;
}


void PolyObject::CreateCylinderGeometry(
	const t_mat& mat,
	t_real rad, t_real height,
	t_real r, t_real g, t_real b)
{
	// 3d object
	auto solid = m::create_cylinder<t_vec3>(rad, height, 1, 32);
	std::tie(m_triangles, m_trianglenorms, m_triangleuvs) =
		m::create_triangles<t_vec3>(solid);

	m_vecVerts = to_float_array(m_triangles, 1, 3, 4, 1.f);
	m_vecNorms = to_float_array(m_trianglenorms, 3, 3, 4, 0.f);
	m_vecUVs = to_float_array(m_triangleuvs, 1, 2, 2, 0.f);
	// TODO
	//if(!m_vecUVs.size())
		m_vecUVs.resize(m_triangles.size()*2);

	m_vecCols.reserve(4*m_triangles.size());
	for(std::size_t iVert=0; iVert<m_triangles.size(); ++iVert)
	{
		m_vecCols.push_back(r); m_vecCols.push_back(g);
		m_vecCols.push_back(b); m_vecCols.push_back(1);
	}

	m_mat = mat;
	//t_mat mat_rot = m_mat * m::rotation<t_mat, t_vec>(
	//	m::create<t_vec>({1,0,0}), m::pi<t_real>*0.5);
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
}
