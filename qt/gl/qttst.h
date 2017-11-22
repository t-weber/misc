/**
 * minimal qt gl example
 * @author Tobias Weber
 * @date Nov-2017
 */

#ifndef __QTTST_H__
#define __QTTST_H__

// ----------------------------------------------------------------------------
// GL versions
//#include <QOpenGLFunctions>
//using qgl_funcs = QOpenGLFunctions;

#include <QOpenGLFunctions_3_3_Core>
using qgl_funcs = QOpenGLFunctions_3_3_Core;

//#include <QOpenGLFunctions_4_5_Core>
//using qgl_funcs = QOpenGLFunctions_4_5_Core;
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

	int m_attrVertex = 0;
	int m_attrVertexColor = 0;
	int m_uniMatrixCam = 0;
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
