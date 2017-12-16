/**
 * minimal qt gl example
 * @author Tobias Weber
 * @date Nov-2017
 * @license: see 'LICENSE' file
 */

#ifndef __QTTST_H__
#define __QTTST_H__

// ----------------------------------------------------------------------------
// GL versions
/*#include <QOpenGLFunctions>
using qgl_funcs = QOpenGLFunctions;
#define _GL_MAJ_VER -1
#define _GL_MIN_VER -1*/

#include <QOpenGLFunctions_3_3_Core>
using qgl_funcs = QOpenGLFunctions_3_3_Core;
#define _GL_MAJ_VER 3
#define _GL_MIN_VER 3

/*#include <QOpenGLFunctions_4_5_Core>
using qgl_funcs = QOpenGLFunctions_4_5_Core;
#define _GL_MAJ_VER 4
#define _GL_MIN_VER 5*/
// ----------------------------------------------------------------------------

#include <QDialog>
#include <QTimer>

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLWidget>
#include <QOpenGLTexture>

#include <QMatrix4x4>
#include <QVector4D>
#include <QVector3D>

#include <memory>
#include <chrono>


class GlWidget : public QOpenGLWidget
{
public:
	using QOpenGLWidget::QOpenGLWidget;

	GlWidget(QWidget *pParent);
	virtual ~GlWidget();

	QPointF GlToScreenCoords(const QVector3D& vec, bool *pVisible=nullptr);

protected:
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void paintGL() override;

	void tick(const std::chrono::milliseconds& ms);

private:
	qgl_funcs *m_pGl = nullptr;
	std::shared_ptr<QOpenGLShaderProgram> m_pShaders;
	std::shared_ptr<QOpenGLBuffer> m_pvertexbuf;
	std::shared_ptr<QOpenGLTexture> m_pTexture;

	QMatrix4x4 m_matPerspective, m_matViewport, m_matCam;

	GLuint m_vertexarr = 0;
	GLint m_attrVertex = -1;
	GLint m_attrVertexColor = -1;
	GLint m_attrTexCoords = -1;
	GLint m_uniMatrixProj = -1, m_uniMatrixCam = -1;
	GLint m_uniImg = -1;

	int m_iScreenDims[2] = { -1, -1 };

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
