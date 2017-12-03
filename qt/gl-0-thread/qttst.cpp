/**
 * minimal qt gl threading example
 * (WARNING: not yet working correctly!)
 * @author Tobias Weber
 * @date Nov-2017
 * @license: see 'LICENSE' file
 *
 * References:
 *  * http://doc.qt.io/qt-5/qopenglwidget.html#details
 *  * https://github.com/qt/qtbase/tree/5.10/examples/opengl/threadedqopenglwidget
 */

#include "qttst.h"

#include <QApplication>
#include <QGridLayout>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>

#include <locale>
#include <iostream>

#include <boost/scope_exit.hpp>
#include <boost/algorithm/string/replace.hpp>
namespace algo = boost::algorithm;


QThread *g_pAppThread = nullptr;

// ----------------------------------------------------------------------------
// error codes: https://www.khronos.org/opengl/wiki/OpenGL_Error
#define LOGGLERR(pGl) { if(auto err = pGl->glGetError(); err != GL_NO_ERROR) \
	std::cerr << "gl error in " << __func__ \
	<< " line " << std::dec <<  __LINE__  << ": " \
	<< std::hex << err << std::endl; }



// ----------------------------------------------------------------------------


GlWidget::GlWidget(QWidget* pParent) : QOpenGLWidget(pParent), m_aux(this)
{
	connect(this, &QOpenGLWidget::aboutToResize, this, &GlWidget::AboutToResize);
	connect(this, &QOpenGLWidget::resized, this, &GlWidget::Resized);
	connect(this, &QOpenGLWidget::aboutToCompose, this, &GlWidget::AboutToCompose);
	connect(this, &QOpenGLWidget::frameSwapped, this, &GlWidget::FrameSwapped);

	connect(this, &GlWidget::SigFrameSwapped, &m_aux, &GlWidgetAux::FrameSwapped);
	doneCurrent();

	m_pThread = std::make_unique<QThread>();
	m_aux.moveToThread(m_pThread.get());
	m_pThread->start();
}


GlWidget::~GlWidget()
{
	m_pThread->quit();
	m_pThread->wait();
}


void GlWidget::init()
{
	// --------------------------------------------------------------------
	// shaders
	// --------------------------------------------------------------------
	std::string strFragShader = R"RAW(
		#version ${GLSL_VERSION}

		in vec4 fragcolor;
		out vec4 outcolor;

		void main()
		{
			//outcolor = vec4(0,0,0,1);
			outcolor = fragcolor;
		}
	)RAW";
	// --------------------------------------------------------------------


	// --------------------------------------------------------------------
	std::string strVertexShader = R"RAW(
		#version ${GLSL_VERSION}
		#define PI 3.1415

		in vec4 vertex;
		in vec4 vertexcolor;
		out vec4 fragcolor;

		uniform mat4 cam = mat4(1.);


		// perspective
		// see: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
		mat4 get_perspective()
		{
			const float n = 0.01;
			const float f = 100.;
			const float c = 1./tan(PI*0.5 * 0.5);
			const float ratio = 4./3.;

			return mat4(
				c*ratio, 0.,    0.,           0.,
				0.,      c,     0.,           0.,
				0.,      0.,    -(n+f)/(n-f), 1.,
				0.,      0.,    2.*n*f/(n-f), 0.);
		}

		/*const*/ mat4 proj = get_perspective();


		void main()
		{
			gl_Position = proj * cam * vertex;
			fragcolor = vertexcolor;
		}
	)RAW";
	// --------------------------------------------------------------------


	std::string strGlsl = std::to_string(_GL_MAJ_VER*100 + _GL_MIN_VER*10);
	algo::replace_all(strFragShader, std::string("${GLSL_VERSION}"), strGlsl);
	algo::replace_all(strVertexShader, std::string("${GLSL_VERSION}"), strGlsl);

	// GL functions
	{
		if constexpr(std::is_same_v<qgl_funcs, QOpenGLFunctions>)
			m_pGl = (qgl_funcs*)QOpenGLContext::currentContext()->functions();
		else
			m_pGl = (qgl_funcs*)QOpenGLContext::currentContext()->versionFunctions<qgl_funcs>();

		if(!m_pGl)
		{
			std::cerr << "No suitable GL interface found." << std::endl;
			return;
		}

		std::cout << __func__ << ": "
			<< (char*)m_pGl->glGetString(GL_VERSION) << ", "
			<< (char*)m_pGl->glGetString(GL_VENDOR) << ", "
			<< (char*)m_pGl->glGetString(GL_RENDERER) << ", "
			<< "glsl: " << (char*)m_pGl->glGetString(GL_SHADING_LANGUAGE_VERSION)
			<< std::endl;
	}
	LOGGLERR(m_pGl);


	// shaders
	{
		m_pShaders = std::make_shared<QOpenGLShaderProgram>(this);
		m_pShaders->addShaderFromSourceCode(QOpenGLShader::Fragment, strFragShader.c_str());
		m_pShaders->addShaderFromSourceCode(QOpenGLShader::Vertex, strVertexShader.c_str());
		//m_pShaders->addShaderFromSourceCode(QOpenGLShader::Geometry, pcGeoShader);

		m_pShaders->link();
		std::string strLog = m_pShaders->log().toStdString();
		if(strLog.size())
			std::cerr << "Shader log: " << strLog << std::endl;

		m_uniMatrixCam = m_pShaders->uniformLocation("cam");
		m_attrVertex = m_pShaders->attributeLocation("vertex");
		m_attrVertexColor = m_pShaders->attributeLocation("vertexcolor");
	}
	LOGGLERR(m_pGl);


	// geometries
	{
		m_pGl->glGenVertexArrays(1, &m_vertexarr);
		m_pvertexbuf = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);

		m_pvertexbuf->create();
		m_pvertexbuf->bind();
		BOOST_SCOPE_EXIT(&m_pvertexbuf)
		{ m_pvertexbuf->release(); }
		BOOST_SCOPE_EXIT_END

		std::vector<GLfloat> vecVerts =
		{
			-0.5, 0., 1., 1.,	// vert
			1., 0., 0., 1.,		// color
			0.5, 0., 2., 1.,	// vert
			0., 1., 0., 1.,		// color
			0.5, 0.5, 1., 1.,	// vert
			0., 0., 1., 1.,		// color

			0., 0., 0., 1.,		// line color
		 };
		 m_pvertexbuf->allocate(vecVerts.data(), vecVerts.size()*sizeof(GLfloat));
	}
	LOGGLERR(m_pGl);

	Resized();
}


void GlWidget::AboutToResize()
{
	m_mutex.lock();
}


void GlWidget::Resized()
{
	BOOST_SCOPE_EXIT(&m_mutex)
	{ m_mutex.unlock(); }
	BOOST_SCOPE_EXIT_END

	if(!m_pGl)
		return;

	const int w = this->width();
	const int h = this->height();
	m_pGl->glViewport(0, 0, w, h);

	std::cerr << std::dec << __func__ << ": w = " << w << ", h = " << h << std::endl;
}


void GlWidget::AboutToCompose()
{
	m_mutex.lock();
}


void GlWidget::FrameSwapped()
{
	// in wrong thread?
	if(QThread::currentThread() != g_pAppThread)
	{
		m_mutex.unlock();
		return;
	}

	GlWidget* pWidget = this;
	{
		pWidget->makeCurrent();
		BOOST_SCOPE_EXIT(pWidget)
		{ pWidget->doneCurrent(); }
		BOOST_SCOPE_EXIT_END

		if(!m_pGl)
			init();
	}

	// move the context to the thread to make makeCurrent and doneCurrent available there
	pWidget->context()->moveToThread(m_pThread.get());
	emit SigFrameSwapped();
	m_mutex.unlock();
}


void GlWidgetAux::FrameSwapped()
{
	GlWidget* pWidget = m_pGlWidget;
	pWidget->m_mutex.lock();

	// in wrong thread?
	if(QThread::currentThread() != pWidget->m_pThread.get())
	{
		pWidget->m_mutex.unlock();
		return;
	}

	pWidget->makeCurrent();
	BOOST_SCOPE_EXIT(pWidget)
	{
		pWidget->doneCurrent();
		pWidget->context()->moveToThread(g_pAppThread);
		QMetaObject::invokeMethod(pWidget, "update", Qt::ConnectionType::QueuedConnection);
		pWidget->m_mutex.unlock();
	}
	BOOST_SCOPE_EXIT_END


	qgl_funcs* pGl = pWidget->m_pGl;
	if(!pGl || !pWidget->context()) return;


	// clear
	pGl->glClearColor(1., 1., 1., 1.);
	pGl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// bind shaders
	pWidget->m_pShaders->bind();
	BOOST_SCOPE_EXIT(pWidget)
	{ pWidget->m_pShaders->release(); }
	BOOST_SCOPE_EXIT_END
	LOGGLERR(pGl);


	// camera
	GLfloat matCam[] =
	{
		1., 0., 0., 0.,
		0., 1., 0., 0.,
		0., 0., 1., 0.,
		0., 0., 0., 1.,
	};
	pGl->glUniformMatrix4fv(pWidget->m_uniMatrixCam, 1, 0, matCam);
	LOGGLERR(pGl);


	// geometry
	if(pWidget->m_pvertexbuf)
	{
		pGl->glBindVertexArray(pWidget->m_vertexarr);

		pGl->glEnableVertexAttribArray(pWidget->m_attrVertex);
		pGl->glEnableVertexAttribArray(pWidget->m_attrVertexColor);
		BOOST_SCOPE_EXIT(pGl, pWidget)
		{
			pGl->glDisableVertexAttribArray(pWidget->m_attrVertexColor);
			pGl->glDisableVertexAttribArray(pWidget->m_attrVertex);
		}
		BOOST_SCOPE_EXIT_END
		LOGGLERR(pGl);

		pWidget->m_pvertexbuf->bind();
		BOOST_SCOPE_EXIT(pWidget)
		{ pWidget->m_pvertexbuf->release(); }
		BOOST_SCOPE_EXIT_END
		LOGGLERR(pGl);

		// triangles
		pGl->glVertexAttribPointer(pWidget->m_attrVertex, 3, GL_FLOAT, 0, 8*sizeof(GLfloat), (void*)0);
		pGl->glVertexAttribPointer(pWidget->m_attrVertexColor, 4, GL_FLOAT, 0, 8*sizeof(GLfloat), (void*)(4*sizeof(GLfloat)));
		pGl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
		LOGGLERR(pGl);

		// lines
		pGl->glVertexAttribPointer(pWidget->m_attrVertexColor, 4, GL_FLOAT, 0, 4*sizeof(GLfloat), (void*)((6*4)*sizeof(GLfloat)));
		pGl->glDrawArrays(GL_LINE_LOOP, 0, 3);
		pGl->glDrawArrays(GL_POINTS, 0, 3);
		LOGGLERR(pGl);
	}
}


// ----------------------------------------------------------------------------




// ----------------------------------------------------------------------------
TstDlg::TstDlg(QWidget* pParent) : QDialog{pParent},
	m_vecGlWidgets
	{{
		std::make_shared<GlWidget>(this),
		std::make_shared<GlWidget>(this),
		std::make_shared<GlWidget>(this),
		std::make_shared<GlWidget>(this)
	}}
{
	auto pGrid = new QGridLayout(this);
	pGrid->setSpacing(2);
	pGrid->setContentsMargins(4,4,4,4);
	pGrid->addWidget(m_vecGlWidgets[0].get(), 0,0,1,1);
	pGrid->addWidget(m_vecGlWidgets[1].get(), 0,1,1,1);
	pGrid->addWidget(m_vecGlWidgets[2].get(), 1,0,1,1);
	pGrid->addWidget(m_vecGlWidgets[3].get(), 1,1,1,1);
}
// ----------------------------------------------------------------------------




// ----------------------------------------------------------------------------
static inline void set_locales()
{
	std::ios_base::sync_with_stdio(false);

	::setlocale(LC_ALL, "C");
	std::locale::global(std::locale("C"));
	QLocale::setDefault(QLocale::C);
}


static inline void set_gl_format(bool bCore=1, int iMajorVer=3, int iMinorVer=3)
{
	QSurfaceFormat surf = QSurfaceFormat::defaultFormat();

	//surf.setOptions(QSurfaceFormat::DebugContext);
	surf.setRenderableType(QSurfaceFormat::OpenGL);
	if(bCore)
		surf.setProfile(QSurfaceFormat::CoreProfile);
	else
		surf.setProfile(QSurfaceFormat::CompatibilityProfile);
	surf.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

	if(iMajorVer > 0 && iMinorVer > 0)
		surf.setVersion(iMajorVer, iMinorVer);

	QSurfaceFormat::setDefaultFormat(surf);
}


int main(int argc, char** argv)
{
	std::unique_ptr<QApplication> pApp = std::make_unique<QApplication>(argc, argv);
	g_pAppThread = pApp->thread();

	set_locales();
	set_gl_format(1, _GL_MAJ_VER, _GL_MIN_VER);

	auto dlg = std::make_unique<TstDlg>(nullptr);
	dlg->resize(800, 600);
	dlg->show();

	return pApp->exec();
}
// ----------------------------------------------------------------------------
