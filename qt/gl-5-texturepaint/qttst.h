/**
 * minimal qt gl example
 * @author Tobias Weber
 * @date Nov-2017
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __QTTST_H__
#define __QTTST_H__

// ----------------------------------------------------------------------------
// GL version to use
#if !defined(_GL_MAJ_VER) || !defined(_GL_MIN_VER)
	#define _GL_MAJ_VER 3
	#define _GL_MIN_VER 3
#endif

// include GL functions
#define _GL_INC_IMPL(MAJ, MIN) <QOpenGLFunctions_ ## MAJ ## _ ## MIN ## _Core>
#define _GL_INC(MAJ, MIN) _GL_INC_IMPL(MAJ, MIN)
#include _GL_INC(_GL_MAJ_VER, _GL_MIN_VER)

// GL functions typedef
#define _GL_FUNC_IMPL(MAJ, MIN) QOpenGLFunctions_ ## MAJ ## _ ## MIN ## _Core
#define _GL_FUNC(MAJ, MIN) _GL_FUNC_IMPL(MAJ, MIN)
using qgl_funcs = _GL_FUNC(_GL_MAJ_VER, _GL_MIN_VER);
// ----------------------------------------------------------------------------

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLWidget>
#include <QOpenGLTexture>

#include <QMatrix4x4>
#include <QVector4D>
#include <QVector3D>

#include <QDialog>
#include <QTimer>
#include <QMouseEvent>

#include <memory>
#include <chrono>

#include "../../libs/math_algos.h"

using t_real = float;
using t_vec3 = m::qvecN_adapter<int, 3, t_real, QVector3D>;
using t_vec = m::qvecN_adapter<int, 4, t_real, QVector4D>;
using t_mat = m::qmatNN_adapter<int, 4, 4, t_real, QMatrix4x4>;


class GlWidget : public QOpenGLWidget
{
public:
	using QOpenGLWidget::QOpenGLWidget;

	GlWidget(QWidget *pParent);
	virtual ~GlWidget();

	QPointF GlToScreenCoords(const t_vec& vec, bool *pVisible=nullptr);

protected:
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void paintGL() override;

	virtual void mouseMoveEvent(QMouseEvent *pEvt) override;

	void tick(const std::chrono::milliseconds& ms);
	void updatePicker();

private:
	qgl_funcs *m_pGl = nullptr;
	std::shared_ptr<QOpenGLShaderProgram> m_pShaders;

	std::shared_ptr<QOpenGLBuffer> m_pvertexbuf;
	std::shared_ptr<QOpenGLBuffer> m_pnormalsbuf;

	std::shared_ptr<QOpenGLBuffer> m_plinebuf;

	std::shared_ptr<QOpenGLTexture> m_pTexture;
	std::shared_ptr<QOpenGLBuffer> m_puvbuf;
	std::shared_ptr<QOpenGLBuffer> m_pcolorbuf;


	t_mat m_matPerspective, m_matPerspective_inv;
	t_mat m_matViewport, m_matViewport_inv;
	t_mat m_matCam, m_matCam_inv;

	std::array<GLuint, 2> m_vertexarr;
	GLint m_attrVertex = -1;
	GLint m_attrVertexNormal = -1;
	GLint m_attrVertexColor = -1;
	GLint m_attrTexCoords = -1;
	GLint m_uniMatrixProj = -1, m_uniMatrixCam = -1;
	GLint m_uniImg = -1;

	// cursor UV coordinates
	GLint m_uniCurUV = -1;
	GLfloat m_curUV[2] = {0., 0.};

	std::vector<t_vec3> m_vertices, m_triangles, m_uvs;
	std::vector<t_vec3> m_lines;

	int m_iScreenDims[2] = { -1, -1 };
	QPointF m_posMouse;

	QTimer m_timer;

protected slots:
	void tick();
};


class TstDlg : public QDialog
{
public:
	using QDialog::QDialog;
	TstDlg(QWidget* pParent);
	~TstDlg() = default;

private:
	std::shared_ptr<GlWidget> m_pGlWidget;
};


#endif
