/**
 * minimal qt gl example
 * @author Tobias Weber
 * @date Nov-2017
 * @license: see 'LICENSE.GPL' file
 *
 * References:
 *  * http://doc.qt.io/qt-5/qopenglwidget.html#details
 *  * http://doc.qt.io/qt-5/qopengltexture.html
 */

#include <locale>
#include <iostream>

#include "qttst.h"

#include <QApplication>
#include <QGridLayout>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>
#include <QPainter>

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

		uniform sampler2D img;
		in vec2 fragtexcoords;

		// cursor position
		uniform vec2 fragCurUV = vec2(0.25, 0.25);

		void main()
		{
			outcolor = texture(img, fragtexcoords);
			outcolor *= fragcolor;

			// paint cursor position
			if(length(fragtexcoords - fragCurUV) < 0.02)
				outcolor = vec4(1,1,1,1);
})RAW";
	// --------------------------------------------------------------------


	// --------------------------------------------------------------------
	std::string strVertexShader = R"RAW(
		#version ${GLSL_VERSION}

		in vec4 vertex;
		in vec4 normal;
		in vec4 vertexcolor;
		out vec4 fragcolor;

		in vec2 texcoords;
		out vec2 fragtexcoords;

		uniform mat4 proj = mat4(1.);
		uniform mat4 cam = mat4(1.);

		vec3 light_dir = vec3(2, 2, -1);

		float lighting(vec3 lightdir)
		{
			float I = dot(vec3(cam*normal), normalize(lightdir));
			if(I < 0) I = 0;
			return I;
		}

		void main()
		{
			gl_Position = proj * cam * vertex;

			float I = lighting(light_dir);
			fragcolor = vertexcolor * I;
			fragcolor[3] = 1;

			fragtexcoords = texcoords;
		})RAW";
	// --------------------------------------------------------------------


	// --------------------------------------------------------------------
	std::string strGeoShader = R"RAW(
		#version ${GLSL_VERSION}

		void main()
		{
		})RAW";
	// --------------------------------------------------------------------


	// set glsl version
	std::string strGlsl = std::to_string(_GL_MAJ_VER*100 + _GL_MIN_VER*10);
	for(std::string* strSrc : { &strFragShader, &strVertexShader, &strGeoShader })
		algo::replace_all(*strSrc, std::string("${GLSL_VERSION}"), strGlsl);


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
		m_uniImg = m_pShaders->uniformLocation("img");
		m_uniCurUV = m_pShaders->uniformLocation("fragCurUV");
		m_attrVertex = m_pShaders->attributeLocation("vertex");
		m_attrVertexNormal = m_pShaders->attributeLocation("normal");
		m_attrVertexColor = m_pShaders->attributeLocation("vertexcolor");
		m_attrTexCoords = m_pShaders->attributeLocation("texcoords");
	}
	LOGGLERR

	// flatten vertex array into raw float array
	auto to_float_array = [](const std::vector<t_vec3>& verts, int iRepeat=1, int iElems=3)
		-> std::vector<GLfloat>
	{
		std::vector<GLfloat> vecRet;
		vecRet.reserve(iRepeat*verts.size()*iElems);

		for(const t_vec3& vert : verts)
			for(int i=0; i<iRepeat; ++i)
				for(int iElem=0; iElem<iElems; ++iElem)
					vecRet.push_back(vert[iElem]);
		return vecRet;
	};

	// geometries
	{
		auto solid = m::create_plane<t_mat, t_vec3>(m::create<t_vec3>({0,0,-1}), 1.5);
		auto [verts, norms, uvs] = m::subdivide_triangles<t_vec3>(m::create_triangles<t_vec3>(solid), 2);
		m_lines = m::create_lines<t_vec3>(std::get<0>(solid), std::get<1>(solid));

		// main vertex array object
		m_pGl->glGenVertexArrays(2, m_vertexarr.data());
		m_pGl->glBindVertexArray(m_vertexarr[0]);

		{	// vertices
			m_pvertexbuf = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);

			m_pvertexbuf->create();
			m_pvertexbuf->bind();
			BOOST_SCOPE_EXIT(&m_pvertexbuf)
			{ m_pvertexbuf->release(); }
			BOOST_SCOPE_EXIT_END

			auto vecVerts = to_float_array(verts);
			m_pvertexbuf->allocate(vecVerts.data(), vecVerts.size()*sizeof(typename decltype(vecVerts)::value_type));
			m_pGl->glVertexAttribPointer(m_attrVertex, 3, GL_FLOAT, 0, 0, (void*)(0*sizeof(typename decltype(vecVerts)::value_type)));
		}

		{	// normals
			m_pnormalsbuf = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);

			m_pnormalsbuf->create();
			m_pnormalsbuf->bind();
			BOOST_SCOPE_EXIT(&m_pnormalsbuf)
			{ m_pnormalsbuf->release(); }
			BOOST_SCOPE_EXIT_END

			auto vecNorms = to_float_array(norms, 3);
			m_pnormalsbuf->allocate(vecNorms.data(), vecNorms.size()*sizeof(typename decltype(vecNorms)::value_type));
			m_pGl->glVertexAttribPointer(m_attrVertexNormal, 3, GL_FLOAT, 0, 0, (void*)(0*sizeof(typename decltype(vecNorms)::value_type)));
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
				vecCols.push_back(0); vecCols.push_back(0);
				vecCols.push_back(1); vecCols.push_back(1);
			}

			m_pcolorbuf->allocate(vecCols.data(), vecCols.size()*sizeof(typename decltype(vecCols)::value_type));
			m_pGl->glVertexAttribPointer(m_attrVertexColor, 4, GL_FLOAT, 0, 0, (void*)(0*sizeof(typename decltype(vecCols)::value_type)));
		}

		{	// texture
			QImage img("/home/tw/tmp/I/0.jpg");
			img = img.mirrored(false, true);
			//QImage img = QImage::fromData(data, size);
			if(!img.isNull())
				m_pTexture = std::make_shared<QOpenGLTexture>(img);
			else
				std::cerr << "Cannot load texture!" << std::endl;

			// uv coords
			m_puvbuf = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);

			m_puvbuf->create();
			m_puvbuf->bind();
			BOOST_SCOPE_EXIT(&m_puvbuf)
			{ if(m_puvbuf) m_puvbuf->release(); }
			BOOST_SCOPE_EXIT_END

			auto vecUVS = to_float_array(uvs, 1, 2);
			m_puvbuf->allocate(vecUVS.data(), vecUVS.size()*sizeof(typename decltype(vecUVS)::value_type));
			m_pGl->glVertexAttribPointer(m_attrTexCoords, 2, GL_FLOAT, 0, 0, (void*)(0*sizeof(typename decltype(vecUVS)::value_type)));
		}


		m_pGl->glBindVertexArray(m_vertexarr[1]);

		{	// lines
			m_plinebuf = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);

			m_plinebuf->create();
			m_plinebuf->bind();
			BOOST_SCOPE_EXIT(&m_plinebuf)
			{ m_plinebuf->release(); }
			BOOST_SCOPE_EXIT_END

			auto vecVerts = to_float_array(m_lines);
			m_plinebuf->allocate(vecVerts.data(), vecVerts.size()*sizeof(typename decltype(vecVerts)::value_type));
			m_pGl->glVertexAttribPointer(m_attrVertex, 3, GL_FLOAT, 0, 0, (void*)(0*sizeof(typename decltype(vecVerts)::value_type)));
		}

		m_vertices = std::move(std::get<0>(solid));
		m_triangles = std::move(verts);
		m_uvs = std::move(uvs);
	}
	LOGGLERR


	// options
	m_pGl->glCullFace(GL_BACK);
	m_pGl->glDisable(GL_CULL_FACE);

	//m_pGl->glEnable(GL_LINE_SMOOTH);
	//m_pGl->glEnable(GL_POLYGON_SMOOTH);
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
		// texture
		m_pShaders->setUniformValue(m_uniImg, 0);
		// cursor
		m_pShaders->setUniformValue(m_uniCurUV, m_curUV[0], m_curUV[1]);


		// triangle geometry
		if(m_pvertexbuf)
		{
			// main vertex array object
			m_pGl->glBindVertexArray(m_vertexarr[0]);

			m_pGl->glEnableVertexAttribArray(m_attrVertex);
			m_pGl->glEnableVertexAttribArray(m_attrVertexNormal);
			m_pGl->glEnableVertexAttribArray(m_attrVertexColor);
			m_pGl->glEnableVertexAttribArray(m_attrTexCoords);
			BOOST_SCOPE_EXIT(m_pGl, &m_attrVertex, &m_attrVertexNormal, &m_attrVertexColor, &m_attrTexCoords)
			{
				m_pGl->glDisableVertexAttribArray(m_attrTexCoords);
				m_pGl->glDisableVertexAttribArray(m_attrVertexColor);
				m_pGl->glDisableVertexAttribArray(m_attrVertexNormal);
				m_pGl->glDisableVertexAttribArray(m_attrVertex);
			}
			BOOST_SCOPE_EXIT_END
			LOGGLERR

			if(m_pTexture) m_pTexture->bind();
			BOOST_SCOPE_EXIT(&m_pTexture)
			{ if(m_pTexture) m_pTexture->release(); }
			BOOST_SCOPE_EXIT_END

			m_pGl->glDrawArrays(GL_TRIANGLES, 0, m_triangles.size());
			LOGGLERR
		}

		// lines
		if(m_plinebuf)
		{
			// auxiliary vertex array object
			m_pGl->glBindVertexArray(m_vertexarr[1]);

			m_pGl->glEnableVertexAttribArray(m_attrVertex);
			BOOST_SCOPE_EXIT(m_pGl, &m_attrVertex)
			{ m_pGl->glDisableVertexAttribArray(m_attrVertex); }
			BOOST_SCOPE_EXIT_END
			LOGGLERR

			m_pGl->glDrawArrays(GL_LINES, 0, m_lines.size());
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

	const GLfloat unsel[] = {1.,1.,1.,1., 1.,1.,1.,1., 1.,1.,1.,1.};
	const GLfloat sel[] = {1.,0.,0.,1., 1.,0.,0.,1., 1.,0.,0.,1.};

	for(std::size_t startidx=0; startidx+2<m_triangles.size(); startidx+=3)
	{
		std::vector<t_vec3> poly{{ m_triangles[startidx+0], m_triangles[startidx+1], m_triangles[startidx+2] }};
		auto [vecInters, bInters, lamInters] =
			m::intersect_line_poly<t_vec3>(
				t_vec3(org[0], org[1], org[2]), t_vec3(dir[0], dir[1], dir[2]), poly);

		if(bInters)
		{
			std::vector<t_vec3> polyuv{{ m_uvs[startidx+0], m_uvs[startidx+1], m_uvs[startidx+2] }};
			//auto uv = m::poly_uv_ortho<t_vec3>(poly[0], poly[1], poly[2], polyuv[0], polyuv[1], polyuv[2], vecInters);
			auto uv = m::poly_uv<t_mat, t_vec3>(poly[0], poly[1], poly[2], polyuv[0], polyuv[1], polyuv[2], vecInters);
			m_curUV[0] = uv[0]; m_curUV[1] = uv[1];

			//std::cout << "Intersection with polygon " << startidx/3 << ", uv: " << uv[0] << ", " << uv[1] << std::endl;
			m_pcolorbuf->write(sizeof(sel[0])*startidx*4, sel, sizeof(sel));
		}
		else
		{
			m_pcolorbuf->write(sizeof(unsel[0])*startidx*4, unsel, sizeof(unsel));
		}
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
