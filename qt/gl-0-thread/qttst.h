/**
 * minimal qt gl threading example
 * @author Tobias Weber
 * @date Nov-2017
 * @license: see 'LICENSE.GPL' file
 *
 * References:
 *  * http://doc.qt.io/qt-5/qopenglwidget.html#details
 *  * https://github.com/qt/qtbase/tree/5.10/examples/opengl/threadedqopenglwidget
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
#include <QThread>
#include <QMutex>

#include <vector>
#include <memory>


class GlWidget;

/**
 * auxiliary object which is moved off-thread
 * only needed because the main widget is not allowed to be moved to another thread
 */
class GlWidgetAux : public QObject
{ Q_OBJECT
public:
	GlWidgetAux(GlWidget* pWid) : m_pGlWidget(pWid)
	{}
	virtual ~GlWidgetAux()
	{}

private:
	GlWidget *m_pGlWidget;

public slots:
	void FrameSwapped();
};


/**
 * main GL widget
 */
class GlWidget : public QOpenGLWidget
{ Q_OBJECT
	friend class GlWidgetAux;

public:
	using QOpenGLWidget::QOpenGLWidget;
	GlWidget(QWidget* pParent);
	virtual ~GlWidget();

protected:
	void init();

private:
	GlWidgetAux m_aux;
	std::unique_ptr<QThread> m_pThread;
	QMutex m_mutex = QMutex(QMutex::Recursive);

	qgl_funcs *m_pGl = nullptr;
	std::shared_ptr<QOpenGLShaderProgram> m_pShaders;
	std::shared_ptr<QOpenGLBuffer> m_pvertexbuf;

	GLuint m_vertexarr = 0;
	GLint m_attrVertex = -1;
	GLint m_attrVertexColor = -1;
	GLint m_uniMatrixCam = -1;

protected: // overrided empty functions
	virtual void initializeGL() override {}
	virtual void resizeGL(int, int) override {}
	virtual void paintGL() override {}
	virtual void paintEvent(QPaintEvent*) override {}
	virtual void resizeEvent(QResizeEvent* pEvt) override { QOpenGLWidget::resizeEvent(pEvt); }

protected slots:
	void AboutToCompose();
	void AboutToResize();
	void FrameSwapped();
	void Resized();

signals:
	void SigFrameSwapped();
};



class TstDlg : public QDialog
{ Q_OBJECT
public:
	using QDialog::QDialog;
	TstDlg(QWidget* pParent);
	~TstDlg() = default;

private:
	std::vector<std::shared_ptr<GlWidget>> m_vecGlWidgets;
};

#endif
