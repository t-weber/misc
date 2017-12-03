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
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLWidget>

#include <memory>


class GlWidget : public QOpenGLWidget
{
public:
	using QOpenGLWidget::QOpenGLWidget;

protected:
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void paintGL() override;

private:
	qgl_funcs *m_pGl = nullptr;
	std::shared_ptr<QOpenGLShaderProgram> m_pShaders;
	std::shared_ptr<QOpenGLBuffer> m_pvertexbuf;

	GLuint m_vertexarr = 0;
	GLint m_attrVertex = -1;
	GLint m_attrVertexColor = -1;
	GLint m_uniMatrixCam = -1;
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
