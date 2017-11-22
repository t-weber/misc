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

#include <locale>
#include <iostream>
#include <boost/scope_exit.hpp>


// ----------------------------------------------------------------------------
void GlWidget::initializeGL()
{
	// shaders
	const char* pcFragShader = R"RAW(
#version 330

in vec4 fragcolor;
out vec4 outcolor;

void main()
{
	//outcolor = vec4(0,0,0,1);
	outcolor = fragcolor;
})RAW";


const char* pcVertexShader = R"RAW(
#version 330
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
})RAW";


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
	}


	// shaders
	{
		m_pShaders = std::make_shared<QOpenGLShaderProgram>(this);
		m_pShaders->addShaderFromSourceCode(QOpenGLShader::Fragment, pcFragShader);
		m_pShaders->addShaderFromSourceCode(QOpenGLShader::Vertex, pcVertexShader);
		//m_pShaders->addShaderFromSourceCode(QOpenGLShader::Geometry, pcGeoShader);

		m_pShaders->link();
		std::string strLog = m_pShaders->log().toStdString();
		if(strLog.size())
			std::cerr << "Shader log: " << strLog << std::endl;

		m_uniMatrixCam = m_pShaders->uniformLocation("cam");
		m_attrVertex = m_pShaders->attributeLocation("vertex");
		m_attrVertexColor = m_pShaders->attributeLocation("vertexcolor");
	}


	// geometries
	{
		m_pvertexbuf = std::make_shared<QOpenGLBuffer>();

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
}


void GlWidget::resizeGL(int w, int h)
{
	if(!m_pGl) return;

	m_pGl->glViewport(0,0,w,h);
	std::cout << "w = " << w << ", h = " << h << std::endl;
}


void GlWidget::paintGL()
{
	if(!m_pGl) return;

	// clear
	m_pGl->glClearColor(1., 1., 1., 1.);
	m_pGl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_pGl->glLineWidth(2.);
	m_pGl->glPointSize(10.);


	// bind shaders
	m_pShaders->bind();
	BOOST_SCOPE_EXIT(m_pShaders)
	{ m_pShaders->release(); }
	BOOST_SCOPE_EXIT_END


	// camera
	GLfloat matCam[] =
	{
		-1., 0., 0., 0.,
		 0., 1., 0., 0.,
		 0., 0., 1., 0.,
		 0., 0., 0., 1.,
	};
	m_pGl->glUniformMatrix4fv(m_uniMatrixCam, 1, 0, matCam);


	if(m_pvertexbuf)
	{
		m_pGl->glEnableVertexAttribArray(m_attrVertex);
		m_pGl->glEnableVertexAttribArray(m_attrVertexColor);
		BOOST_SCOPE_EXIT(m_pGl, &m_attrVertex, &m_attrVertexColor)
		{
			m_pGl->glEnableVertexAttribArray(m_attrVertexColor);
			m_pGl->glDisableVertexAttribArray(m_attrVertex);
		}
		BOOST_SCOPE_EXIT_END

		m_pvertexbuf->bind();
		BOOST_SCOPE_EXIT(&m_pvertexbuf)
		{ m_pvertexbuf->release(); }
		BOOST_SCOPE_EXIT_END

		// direct drawing without buffer: m_pGl->glVertexAttribPointer
		// triangles
		m_pShaders->setAttributeBuffer(m_attrVertex, GL_FLOAT, 0, 3, 8*sizeof(GLfloat));
		m_pShaders->setAttributeBuffer(m_attrVertexColor, GL_FLOAT, 4*sizeof(GLfloat), 4, 8*sizeof(GLfloat));
		m_pGl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

		// lines
		m_pShaders->setAttributeBuffer(m_attrVertexColor, GL_FLOAT, (6*4)*sizeof(GLfloat), 4, 4*sizeof(GLfloat));
		m_pGl->glDrawArrays(GL_LINE_LOOP, 0, 3);
		m_pGl->glDrawArrays(GL_POINTS, 0, 3);
	}
}
// ----------------------------------------------------------------------------




// ----------------------------------------------------------------------------
TstDlg::TstDlg(QWidget* pParent) : QDialog{pParent},
	m_pGlWidget{new GlWidget(this)}
{
	auto pGrid = new QGridLayout(this);
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


int main(int argc, char** argv)
{
	auto app = std::make_unique<QApplication>(argc, argv);
	set_locales();

	auto dlg = std::make_unique<TstDlg>(nullptr);
	dlg->resize(800, 600);
	dlg->show();

	return app->exec();
}
// ----------------------------------------------------------------------------
