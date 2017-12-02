/**
 * minimal qt gl example
 * @author Tobias Weber
 * @date Nov-2017
 *
 * References:
 *  * http://doc.qt.io/qt-5/qopenglwidget.html#details
 */
#include "qttst.h"

#include <QApplication>
#include <QGridLayout>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>
#include <QPainter>

#include <locale>
#include <iostream>

#include <boost/scope_exit.hpp>
#include <boost/algorithm/string/replace.hpp>
namespace algo = boost::algorithm;


// ----------------------------------------------------------------------------
// error codes: https://www.khronos.org/opengl/wiki/OpenGL_Error
#define LOGGLERR { if(auto err = m_pGl->glGetError(); err != GL_NO_ERROR) \
	std::cerr << "gl error in " << __func__ \
	<< " line " << std::dec <<  __LINE__  << ": " \
	<< std::hex << err << std::endl; }


GlWidget::GlWidget(QWidget *pParent)
{
	connect(&m_timer, &QTimer::timeout, this, static_cast<void (GlWidget::*)()>(&GlWidget::tick));
	m_timer.start(std::chrono::milliseconds(1000 / 60));
}


GlWidget::~GlWidget()
{
	m_timer.stop();
}


void GlWidget::initializeGL()
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
		})RAW";
	// --------------------------------------------------------------------


	// --------------------------------------------------------------------
	std::string strVertexShader = R"RAW(
		#version ${GLSL_VERSION}
		#define PI 3.1415

		in vec4 vertex;
		in vec4 vertexcolor;
		out vec4 fragcolor;

		uniform mat4 proj = mat4(1.);
		uniform mat4 cam = mat4(1.);

		void main()
		{
			gl_Position = proj * cam * vertex;
			fragcolor = vertexcolor;
		})RAW";
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
	LOGGLERR


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
		m_uniMatrixProj = m_pShaders->uniformLocation("proj");
		m_attrVertex = m_pShaders->attributeLocation("vertex");
		m_attrVertexColor = m_pShaders->attributeLocation("vertexcolor");
	}
	LOGGLERR


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
			-0.5, 0., -1., 1.,	// vert
			1., 0., 0., 1.,		// color
			0.5, 0., -2., 1.,	// vert
			0., 1., 0., 1.,		// color
			0.5, 0.5, -1., 1.,	// vert
			0., 0., 1., 1.,		// color

			0., 0., 0., 1.,		// line color
		 };
		 m_pvertexbuf->allocate(vecVerts.data(), vecVerts.size()*sizeof(GLfloat));
	}
	LOGGLERR
}


void GlWidget::resizeGL(int w, int h)
{
	m_iScreenDims[0] = w;
	m_iScreenDims[1] = h;

	std::cerr << std::dec << __func__ << ": w = " << w << ", h = " << h << std::endl;
	if(!m_pGl) return;

	m_matViewport.setToIdentity();
	m_matViewport.viewport(0, 0, w, h, 0., 1.);
	m_pGl->glViewport(0, 0, w, h);

	m_matPerspective.setToIdentity();
	m_matPerspective.perspective(90., double(w)/double(h), 0.01, 100.);


	// bind shaders
	m_pShaders->bind();
	BOOST_SCOPE_EXIT(m_pShaders)
	{ m_pShaders->release(); }
	BOOST_SCOPE_EXIT_END
	LOGGLERR

	// set matrices
	m_pShaders->setUniformValue(m_uniMatrixCam, m_matCam);
	m_pShaders->setUniformValue(m_uniMatrixProj, m_matPerspective);
	LOGGLERR
}


void GlWidget::paintGL()
{
	if(!m_pGl) return;
	QPainter painter(this);


	// gl painting
	{
		painter.beginNativePainting();
		BOOST_SCOPE_EXIT(&painter)
		{ painter.endNativePainting(); }
		BOOST_SCOPE_EXIT_END

		// clear
		m_pGl->glClearColor(1., 1., 1., 1.);
		m_pGl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// bind shaders
		m_pShaders->bind();
		BOOST_SCOPE_EXIT(m_pShaders)
		{ m_pShaders->release(); }
		BOOST_SCOPE_EXIT_END
		LOGGLERR


		// set cam matrix
		m_pShaders->setUniformValue(m_uniMatrixCam, m_matCam);


		// geometry
		if(m_pvertexbuf)
		{
			m_pGl->glBindVertexArray(m_vertexarr);

			m_pGl->glEnableVertexAttribArray(m_attrVertex);
			m_pGl->glEnableVertexAttribArray(m_attrVertexColor);
			BOOST_SCOPE_EXIT(m_pGl, &m_attrVertex, &m_attrVertexColor)
			{
				m_pGl->glDisableVertexAttribArray(m_attrVertexColor);
				m_pGl->glDisableVertexAttribArray(m_attrVertex);
			}
			BOOST_SCOPE_EXIT_END
			LOGGLERR

			m_pvertexbuf->bind();
			BOOST_SCOPE_EXIT(&m_pvertexbuf)
			{ m_pvertexbuf->release(); }
			BOOST_SCOPE_EXIT_END
			LOGGLERR

			// triangles
			m_pGl->glVertexAttribPointer(m_attrVertex, 3, GL_FLOAT, 0, 8*sizeof(GLfloat), (void*)0);
			m_pGl->glVertexAttribPointer(m_attrVertexColor, 4, GL_FLOAT, 0, 8*sizeof(GLfloat), (void*)(4*sizeof(GLfloat)));
			m_pGl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
			LOGGLERR

			// lines
			m_pGl->glVertexAttribPointer(m_attrVertexColor, 4, GL_FLOAT, 0, 4*sizeof(GLfloat), (void*)((6*4)*sizeof(GLfloat)));
			m_pGl->glDrawArrays(GL_LINE_LOOP, 0, 3);
			m_pGl->glDrawArrays(GL_POINTS, 0, 3);
			LOGGLERR
		}
	}


	// classic painting
	{
		painter.drawText(GlToScreenCoords(QVector3D(-0.5,0.,-1.)), "* Vertex 1");
		painter.drawText(GlToScreenCoords(QVector3D(0.5, 0., -2.)), "* Vertex 2");
		painter.drawText(GlToScreenCoords(QVector3D(0.5, 0.5, -1.)), "* Vertex 3");
	}
}


void GlWidget::tick()
{
	tick(std::chrono::milliseconds(1000 / 60));
}

void GlWidget::tick(const std::chrono::milliseconds& ms)
{
	m_matCam.rotate(1.5, 0.,0.,1.);
	update();
}


QPointF GlWidget::GlToScreenCoords(const QVector3D& vec3, bool *pVisible)
{
	// homogeneous vector
	QVector4D vec4{vec3};
	vec4[3] = 1;

	// perspective trafo and divide
	QVector4D vecPersp = m_matPerspective * m_matCam * vec4;
	vecPersp /= vecPersp[3];

	// position not visible -> return a point outside the viewport
	if(vecPersp[2] > 1.)
	{
		if(pVisible) *pVisible = false;
		return QPointF(-1*m_iScreenDims[0], -1*m_iScreenDims[1]);
	}

	// viewport trafo
	QVector4D vec = m_matViewport * vecPersp;

	// transform to QPainter coord system
	vec[1] = -vec[1] + m_iScreenDims[1];

	if(pVisible) *pVisible = true;
	return QPointF(vec[0], vec[1]);
}
// ----------------------------------------------------------------------------




// ----------------------------------------------------------------------------
TstDlg::TstDlg(QWidget* pParent) : QDialog{pParent},
	m_pGlWidget{new GlWidget(this)}
{
	auto pGrid = new QGridLayout(this);
	pGrid->setSpacing(2);
	pGrid->setContentsMargins(4,4,4,4);
	pGrid->addWidget(m_pGlWidget.get(), 0,0,1,1);
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
	auto app = std::make_unique<QApplication>(argc, argv);
	set_locales();
	set_gl_format(1, _GL_MAJ_VER, _GL_MIN_VER);

	auto dlg = std::make_unique<TstDlg>(nullptr);
	dlg->resize(800, 600);
	dlg->show();

	return app->exec();
}
// ----------------------------------------------------------------------------
