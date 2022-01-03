/**
 * 3d object
 * @author Tobias Weber
 * @date Feb-2021
 * @license see 'LICENSE.GPL' file
 */

#ifndef __QTVKTST_OBJ_H__
#define __QTVKTST_OBJ_H__

#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QVector4D>
#include <QVector3D>
#include <QVector2D>

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


class PolyObject
{
private:
	std::vector<t_real> m_vecVerts, m_vecNorms, m_vecCols, m_vecUVs;
	std::vector<t_vec3> m_triangles, m_trianglenorms, m_triangleuvs;
	t_mat m_mat{m::unit<t_mat>(4)};
	std::size_t m_mem_offs = 0;

public:
	std::size_t GetNumVertexBufferElements() const;
	std::size_t GetNumVertices() const;

	const t_vec3& GetVertex(std::size_t i) const;
	const t_vec3& GetUV(std::size_t i) const;

	void CreatePlaneGeometry(
		const t_mat& mat,
		const t_vec3& norm=m::create<t_vec3>({0,0,-1}), t_real size=1.5,
		t_real r=0, t_real g=0, t_real b=1);
	void CreateCubeGeometry(
		const t_mat& mat,
		t_real size=1.,
		t_real r=0, t_real g=0, t_real b=1);
	void CreateSphereGeometry(
		const t_mat& mat,
		t_real rad=1.,
		t_real r=0, t_real g=0, t_real b=1);
	void CreateCylinderGeometry(
		const t_mat& mat,
		t_real rad=1., t_real height=1.,
		t_real r=0, t_real g=0, t_real b=1);

	std::size_t UpdateVertexBuffers(t_real* pMem, std::size_t mem_offs);
	std::size_t GetMemOffset() const;

	void SetMatrix(const t_mat& mat);
	const t_mat& GetMatrix() const;

	void tick(const std::chrono::milliseconds& ms);
};


#endif
