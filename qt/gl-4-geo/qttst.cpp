/**
 * minimal qt gl example
 * @author Tobias Weber
 * @date Nov-2017
 * @license: see 'LICENSE' file
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
	setMouseTracking(true);
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
			outcolor = vec4(0,0,0,1);
			outcolor = fragcolor;
		})RAW";
	// --------------------------------------------------------------------


	// --------------------------------------------------------------------
	std::string strVertexShader = R"RAW(
		#version ${GLSL_VERSION}

		in vec4 vertex;
		in vec3 normal;
		in vec4 vertexcolor;
		out vec4 fragcolor;

		uniform mat4 proj = mat4(1.);
		uniform mat4 cam = mat4(1.);

		//vec4 vertexcolor = vec4(0, 0, 1, 1);
		vec3 light_dir = vec3(1, 0.5, 0.25);


		float lighting(vec3 lightdir)
		{
			float I = dot(normal, normalize(lightdir));
			I = abs(I);
			return I;
		}

		void main()
		{
			gl_Position = proj * cam * vertex;

			float I = lighting(light_dir);
			fragcolor = vertexcolor * I;
			fragcolor[3] = 1;
		})RAW";
	// --------------------------------------------------------------------


	// --------------------------------------------------------------------
	std::string strGeoShader = R"RAW(
		#version ${GLSL_VERSION}

		void main()
		{
		})RAW";
	// --------------------------------------------------------------------


	std::string strGlsl = std::to_string(_GL_MAJ_VER*100 + _GL_MIN_VER*10);
	algo::replace_all(strFragShader, std::string("${GLSL_VERSION}"), strGlsl);
	algo::replace_all(strVertexShader, std::string("${GLSL_VERSION}"), strGlsl);
	algo::replace_all(strGeoShader, std::string("${GLSL_VERSION}"), strGlsl);


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
		// shader compiler/linker error handler
		auto shader_err = [this](const char* err) -> void
		{
			std::cerr << err << std::endl;

			std::string strLog = m_pShaders->log().toStdString();
			if(strLog.size())
				std::cerr << "Shader log: " << strLog << std::endl;

			std::exit(-1);
		};

		// compile & link shaders
		m_pShaders = std::make_shared<QOpenGLShaderProgram>(this);

		if(!m_pShaders->addShaderFromSourceCode(QOpenGLShader::Fragment, strFragShader.c_str()))
			shader_err("Cannot compile fragment shader.");
		if(!m_pShaders->addShaderFromSourceCode(QOpenGLShader::Vertex, strVertexShader.c_str()))
			shader_err("Cannot compile vertex shader.");
		//if(!m_pShaders->addShaderFromSourceCode(QOpenGLShader::Geometry, strGeoShader.c_str()))
		//	shader_err("Cannot compile geometry shader.");

		if(!m_pShaders->link())
			shader_err("Cannot link shaders.");

		m_uniMatrixCam = m_pShaders->uniformLocation("cam");
		m_uniMatrixProj = m_pShaders->uniformLocation("proj");
		m_attrVertex = m_pShaders->attributeLocation("vertex");
		m_attrVertexNormal = m_pShaders->attributeLocation("normal");
		m_attrVertexColor = m_pShaders->attributeLocation("vertexcolor");
	}
	LOGGLERR


	// geometries
	{
		auto cube = m::create_cube<t_vec3>(1.);
		auto [verts, norms, uvs] =
			m::subdivide_triangles<t_vec3>(m::subdivide_triangles<t_vec3>(
				m::create_triangles<t_vec3>(cube)));

		// main vertex array object
		m_pGl->glGenVertexArrays(1, &m_vertexarr);
		m_pGl->glBindVertexArray(m_vertexarr);

		{	// vertices
			m_pvertexbuf = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);

			m_pvertexbuf->create();
			m_pvertexbuf->bind();
			BOOST_SCOPE_EXIT(&m_pvertexbuf)
			{ m_pvertexbuf->release(); }
			BOOST_SCOPE_EXIT_END

			std::vector<GLfloat> vecVerts;
			vecVerts.reserve(verts.size()*3);
			for(const t_vec& vert : verts)
			{
				vecVerts.push_back(vert[0]);
				vecVerts.push_back(vert[1]);
				vecVerts.push_back(vert[2]);
			}

			m_pvertexbuf->allocate(vecVerts.data(), vecVerts.size()*sizeof(GLfloat));
			m_pGl->glVertexAttribPointer(m_attrVertex, 3, GL_FLOAT, 0, 0, (void*)(0*sizeof(GLfloat)));
		}

		{	// normals
			m_pnormalsbuf = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);

			m_pnormalsbuf->create();
			m_pnormalsbuf->bind();
			BOOST_SCOPE_EXIT(&m_pnormalsbuf)
			{ m_pnormalsbuf->release(); }
			BOOST_SCOPE_EXIT_END

			std::vector<GLfloat> vecNorms;
			vecNorms.reserve(3*norms.size()*3);
			for(const t_vec& norm : norms)
			{
				// 3 vertices per triangle
				for(int i=0; i<3; ++i)
				{
					vecNorms.push_back(norm[0]);
					vecNorms.push_back(norm[1]);
					vecNorms.push_back(norm[2]);
				}
			}

			m_pnormalsbuf->allocate(vecNorms.data(), vecNorms.size()*sizeof(GLfloat));
			m_pGl->glVertexAttribPointer(m_attrVertexNormal, 3, GL_FLOAT, 0, 0, (void*)(0*sizeof(GLfloat)));
		}

		{	// colors
			m_pcolorbuf = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);

			m_pcolorbuf->create();
			m_pcolorbuf->bind();
			BOOST_SCOPE_EXIT(&m_pcolorbuf)
			{ m_pcolorbuf->release(); }
			BOOST_SCOPE_EXIT_END

			std::vector<GLfloat> vecCols;
			vecCols.reserve(4*verts.size());
			for(std::size_t iVert=0; iVert<verts.size(); ++iVert)
			{
				vecCols.push_back(0);
				vecCols.push_back(0);
				vecCols.push_back(1);
				vecCols.push_back(1);
			}

			m_pcolorbuf->allocate(vecCols.data(), vecCols.size()*sizeof(GLfloat));
			m_pGl->glVertexAttribPointer(m_attrVertexColor, 4, GL_FLOAT, 0, 0, (void*)(0*sizeof(GLfloat)));
		}

		m_vertices = std::move(std::get<0>(cube));
		m_triangles = std::move(verts);
	}
	LOGGLERR
}


void GlWidget::resizeGL(int w, int h)
{
	m_iScreenDims[0] = w;
	m_iScreenDims[1] = h;

	std::cerr << std::dec << __func__ << ": w = " << w << ", h = " << h << std::endl;
	if(!m_pGl) return;

	m_matViewport = m::hom_viewport<t_mat>(w, h, 0., 1.);
	std::tie(m_matViewport_inv, std::ignore) = m::inv<t_mat>(m_matViewport);


	m_pGl->glViewport(0, 0, w, h);
	m_pGl->glDepthRange(0, 1);

	m_matPerspective = m::hom_perspective<t_mat>(0.01, 100., m::pi<t_real>*0.5, t_real(h)/t_real(w));
	std::tie(m_matPerspective_inv, std::ignore) = m::inv<t_mat>(m_matPerspective);


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
		m_pGl->glEnable(GL_DEPTH_TEST);


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
			// main vertex array object
			m_pGl->glBindVertexArray(m_vertexarr);

			m_pGl->glEnableVertexAttribArray(m_attrVertex);
			m_pGl->glEnableVertexAttribArray(m_attrVertexNormal);
			m_pGl->glEnableVertexAttribArray(m_attrVertexColor);
			BOOST_SCOPE_EXIT(m_pGl, &m_attrVertex, &m_attrVertexNormal, &m_attrVertexColor)
			{
				m_pGl->glDisableVertexAttribArray(m_attrVertexColor);
				m_pGl->glDisableVertexAttribArray(m_attrVertexNormal);
				m_pGl->glDisableVertexAttribArray(m_attrVertex);
			}
			BOOST_SCOPE_EXIT_END
			LOGGLERR

			m_pGl->glDrawArrays(GL_TRIANGLES, 0, m_triangles.size());
			LOGGLERR
		}
	}


	// classic painting
	{
		m_pGl->glDisable(GL_DEPTH_TEST);

		std::size_t i = 0;
		for(const auto& vert : m_vertices)
		{
			std::string strName = "* " + std::to_string(i);
			painter.drawText(GlToScreenCoords(t_vec(vert[0], vert[1], vert[2], 1)), strName.c_str());
			++i;
		}
	}
}


void GlWidget::tick()
{
	tick(std::chrono::milliseconds(1000 / 60));
}

void GlWidget::tick(const std::chrono::milliseconds& ms)
{
	static t_real fAngle = 0.f;
	fAngle += 0.5f;

	m_matCam = m::create<t_mat>({1,0,0,0,  0,1,0,0,  0,0,1,-3,  0,0,0,1});
	m_matCam *= m::rotation<t_mat, t_vec>(m::create<t_vec>({1.,1.,0.,0.}), fAngle/180.*M_PI, 0);
	std::tie(m_matCam_inv, std::ignore) = m::inv<t_mat>(m_matCam);

	updatePicker();
	update();
}


QPointF GlWidget::GlToScreenCoords(const t_vec& vec4, bool *pVisible)
{
	auto [ vecPersp, vec ] =
		m::hom_to_screen_coords<t_mat, t_vec>(vec4, m_matCam, m_matPerspective, m_matViewport, true);

	// position not visible -> return a point outside the viewport
	if(vecPersp[2] > 1.)
	{
		if(pVisible) *pVisible = false;
		return QPointF(-1*m_iScreenDims[0], -1*m_iScreenDims[1]);
	}

	if(pVisible) *pVisible = true;
	return QPointF(vec[0], vec[1]);
}


void GlWidget::mouseMoveEvent(QMouseEvent *pEvt)
{
	m_posMouse = pEvt->localPos();
	updatePicker();
}


void GlWidget::updatePicker()
{
	if(!m_pcolorbuf) return;

	m_pcolorbuf->bind();
	BOOST_SCOPE_EXIT(&m_pcolorbuf)
	{ m_pcolorbuf->release(); }
	BOOST_SCOPE_EXIT_END
	LOGGLERR

	auto [org, dir] = m::hom_line_from_screen_coords<t_mat, t_vec>(m_posMouse.x(), m_posMouse.y(), 0., 1.,
		m_matCam_inv, m_matPerspective_inv, m_matViewport_inv, &m_matViewport, true);

	GLfloat red[] = {1.,0.,0.,1., 1.,0.,0.,1., 1.,0.,0.,1.};
	GLfloat blue[] = {0.,0.,1.,1., 0.,0.,1.,1., 0.,0.,1.,1.};

	for(std::size_t startidx=0; startidx+2<m_triangles.size(); startidx+=3)
	{
		std::vector<t_vec3> poly{{ m_triangles[startidx+0], m_triangles[startidx+1], m_triangles[startidx+2] }};
		auto [vecInters, bInters, lamInters] =
			m::intersect_line_poly<t_vec3>(
				t_vec3(org[0], org[1], org[2]), t_vec3(dir[0], dir[1], dir[2]), poly);
		//if(bInters)
		//	std::cout << "Intersection with polygon " << startidx/3 << std::endl;

		if(bInters)
			m_pcolorbuf->write(sizeof(GLfloat)*startidx*4, red, sizeof(red));
		else
			m_pcolorbuf->write(sizeof(GLfloat)*startidx*4, blue, sizeof(blue));
	}
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
