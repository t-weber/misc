/**
 * GL plotter
 * @author Tobias Weber
 * @date Nov-2017
 * @license: see 'LICENSE.GPL' file
 *
 *  * References:
 *  * http://doc.qt.io/qt-5/qopenglwidget.html#details
 *  * http://code.qt.io/cgit/qt/qtbase.git/tree/examples/opengl/threadedqopenglwidget
 */

#ifndef __MAG_GL_PLOT_H__
#define __MAG_GL_PLOT_H__


// ----------------------------------------------------------------------------
// GL versions
#if !defined(_GL_MAJ_VER) || !defined(_GL_MIN_VER)
	#define _GL_MAJ_VER 3
	#define _GL_MIN_VER 3
#endif

// GL functions include
#define _GL_INC_IMPL(MAJ, MIN) <QtGui/QOpenGLFunctions_ ## MAJ ## _ ## MIN ## _Core>
#define _GL_INC(MAJ, MIN) _GL_INC_IMPL(MAJ, MIN)
#include _GL_INC(_GL_MAJ_VER, _GL_MIN_VER)

// GL functions typedef
#define _GL_FUNC_IMPL(MAJ, MIN) QOpenGLFunctions_ ## MAJ ## _ ## MIN ## _Core
#define _GL_FUNC(MAJ, MIN) _GL_FUNC_IMPL(MAJ, MIN)
using qgl_funcs = _GL_FUNC(_GL_MAJ_VER, _GL_MIN_VER);

// GL surface format
extern void set_gl_format(bool bCore=true, int iMajorVer=3, int iMinorVer=3);
// ----------------------------------------------------------------------------

#define _GL_USE_TIMER 0


#include <QtCore/QTimer>
#include <QtWidgets/QDialog>
#include <QtGui/QMouseEvent>

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLBuffer>
#include <QtWidgets/QOpenGLWidget>

#include <QtGui/QMatrix4x4>
#include <QtGui/QVector4D>
#include <QtGui/QVector3D>

#include <memory>
#include <chrono>
#include <atomic>
#include "../../libs/math_algos.h"

using t_real_gl = GLfloat;
using t_vec3_gl = m::qvecN_adapter<int, 3, t_real_gl, QVector3D>;
using t_vec_gl = m::qvecN_adapter<int, 4, t_real_gl, QVector4D>;
using t_mat_gl = m::qmatNN_adapter<int, 4, 4, t_real_gl, QMatrix4x4>;


class GlPlot;


enum class GlPlotObjType
{
	TRIANGLES,
	LINES
};


struct GlPlotObj
{ friend class GlPlot;
private:
	GlPlotObjType m_type = GlPlotObjType::TRIANGLES;
	GLuint m_vertexarr = 0;

	std::shared_ptr<QOpenGLBuffer> m_pvertexbuf;
	std::shared_ptr<QOpenGLBuffer> m_pnormalsbuf;
	std::shared_ptr<QOpenGLBuffer> m_pcolorbuf;

	std::vector<t_vec3_gl> m_vertices, m_triangles;
	t_vec_gl m_color = m::create<t_vec_gl>({ 0., 0., 1., 1. });	// rgba

	t_mat_gl m_mat = m::unit<t_mat_gl>();

	bool m_visible = true;		// object shown?
	//std::vector<t_vec3_gl> m_pickerInters;		// intersections with mouse picker?

	t_vec3_gl m_labelPos = m::create<t_vec3_gl>({0., 0., 0.});
	std::string m_label;

public:
	GlPlotObj() = default;
	~GlPlotObj() = default;
};



class GlPlot : public QOpenGLWidget
{ Q_OBJECT
public:
	using QOpenGLWidget::QOpenGLWidget;

	GlPlot(QWidget *pParent);
	virtual ~GlPlot();

protected:
	virtual void paintGL() override;
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;

	virtual void mouseMoveEvent(QMouseEvent *pEvt) override;
	virtual void mousePressEvent(QMouseEvent *Evt) override;
	virtual void mouseReleaseEvent(QMouseEvent *Evt) override;
	virtual void wheelEvent(QWheelEvent *pEvt) override;


	qgl_funcs* GetGlFunctions();

	void tick(const std::chrono::milliseconds& ms);
	void UpdatePicker();
	void UpdateCam();

	GlPlotObj CreateTriangleObject(const std::vector<t_vec3_gl>& verts,
		const std::vector<t_vec3_gl>& triag_verts, const std::vector<t_vec3_gl>& norms,
		const t_vec_gl& color, bool bUseVertsAsNorm=false);
	GlPlotObj CreateLineObject(const std::vector<t_vec3_gl>& verts, const t_vec_gl& color);

private:
	std::atomic<bool> m_bPickerNeedsUpdate = false;
	std::string m_strGlDescr;

	std::vector<GlPlotObj> m_objs;

	std::shared_ptr<QOpenGLShaderProgram> m_pShaders;

	t_mat_gl m_matPerspective, m_matPerspective_inv;
	t_mat_gl m_matViewport, m_matViewport_inv;
	t_mat_gl m_matCamBase, m_matCamRot;
	t_mat_gl m_matCam, m_matCam_inv;
	t_vec_gl m_vecCamX, m_vecCamY;
	t_real_gl m_phi_saved = 0, m_theta_saved = 0;
	t_real_gl m_zoom = 1.;

	GLint m_attrVertex = -1;
	GLint m_attrVertexNormal = -1;
	GLint m_attrVertexColor = -1;
	GLint m_uniMatrixProj = -1;
	GLint m_uniMatrixCam = -1;
	GLint m_uniMatrixObj = -1;

	std::atomic<int> m_iScreenDims[2] = { 800, 600 };
	QPointF m_posMouse;
	QPointF m_posMouseRotationStart, m_posMouseRotationEnd;
	bool m_bInRotation = false;

	#if _GL_USE_TIMER != 0
	QTimer m_timer;
	#endif

public:
	std::size_t AddSphere(t_real_gl rad=1,
		t_real_gl x=0, t_real_gl y=0, t_real_gl z=0,
		t_real_gl r=0, t_real_gl g=0, t_real_gl b=0, t_real_gl a=1);
	std::size_t AddCylinder(t_real_gl rad=1, t_real_gl h=1,
		t_real_gl x=0, t_real_gl y=0, t_real_gl z=0,
		t_real_gl r=0, t_real_gl g=0, t_real_gl b=0, t_real_gl a=1);
	std::size_t AddCone(t_real_gl rad=1, t_real_gl h=1,
		t_real_gl x=0, t_real_gl y=0, t_real_gl z=0,
		t_real_gl r=0, t_real_gl g=0, t_real_gl b=0, t_real_gl a=1);
	std::size_t AddArrow(t_real_gl rad=1, t_real_gl h=1,
		t_real_gl x=0, t_real_gl y=0, t_real_gl z=0,
		t_real_gl r=0, t_real_gl g=0, t_real_gl b=0, t_real_gl a=1);
	std::size_t AddCoordinateCross(t_real_gl min, t_real_gl max);

	static t_mat_gl GetArrowMatrix(const t_vec_gl& vecTo, t_real_gl scale,
		const t_vec_gl& vecTrans = m::create<t_vec_gl>({0,0,0.5}), const t_vec_gl& vecFrom = m::create<t_vec_gl>({0,0,1}));

	void SetObjectMatrix(std::size_t idx, const t_mat_gl& mat);
	void SetObjectLabel(std::size_t idx, const std::string& label);
	void SetObjectVisible(std::size_t idx, bool visible);

	const std::string& GetGlDescr() const { return m_strGlDescr; }

	void SetCamBase(const t_mat_gl& mat, const t_vec_gl& vecX, const t_vec_gl& vecY)
	{ m_matCamBase = mat; m_vecCamX = vecX; m_vecCamY = vecY; UpdateCam(); }


	QPointF GlToScreenCoords(const t_vec_gl& vec, bool *pVisible=nullptr);

protected slots:
	void tick();

	void zoom(t_real_gl val);
	void ResetZoom();

	void BeginRotation();
	void EndRotation();

signals:
	void PickerIntersection(const t_vec3_gl* pos, std::size_t objIdx, const t_vec3_gl* posSphere);
	void AfterGLInitialisation();
};


#endif
