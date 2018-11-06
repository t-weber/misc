/**
 * GL plotter
 * @author Tobias Weber
 * @date Nov-2017
 * @license: see 'LICENSE.GPL' file
 *
 * References:
 *  * http://doc.qt.io/qt-5/qopenglwidget.html#details
 *  * http://code.qt.io/cgit/qt/qtbase.git/tree/examples/opengl/threadedqopenglwidget
 */

#include "glplot.h"

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QPainter>
#include <QtGui/QGuiApplication>
#include <QtCore/QMutex>

#include <iostream>
#include <boost/scope_exit.hpp>
#include <boost/algorithm/string/replace.hpp>
namespace algo = boost::algorithm;



// ----------------------------------------------------------------------------
void set_gl_format(bool bCore, int iMajorVer, int iMinorVer)
{
	QSurfaceFormat surf = QSurfaceFormat::defaultFormat();

	surf.setRenderableType(QSurfaceFormat::OpenGL);
	if(bCore)
		surf.setProfile(QSurfaceFormat::CoreProfile);
	else
		surf.setProfile(QSurfaceFormat::CompatibilityProfile);

	if(iMajorVer > 0 && iMinorVer > 0)
		surf.setVersion(iMajorVer, iMinorVer);

	surf.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	surf.setSamples(8);	// multisampling

	QSurfaceFormat::setDefaultFormat(surf);
}


// error codes: https://www.khronos.org/opengl/wiki/OpenGL_Error
#define LOGGLERR(pGl) { if(auto err = pGl->glGetError(); err != GL_NO_ERROR) \
	std::cerr << "gl error in " << __func__ \
		<< " line " << std::dec <<  __LINE__  << ": " \
		<< std::hex << err << std::endl; }
// ----------------------------------------------------------------------------


GlPlot::GlPlot(QWidget *pParent) : QOpenGLWidget(pParent),
	m_matPerspective{m::unit<t_mat_gl>()}, m_matPerspective_inv{m::unit<t_mat_gl>()},
	m_matViewport{m::unit<t_mat_gl>()}, m_matViewport_inv{m::unit<t_mat_gl>()},
	m_matCamBase(m::create<t_mat_gl>({1,0,0,0,  0,1,0,0,  0,0,1,-5,  0,0,0,1})),
	m_matCamRot{m::unit<t_mat_gl>()},
	m_matCam{m::unit<t_mat_gl>()}, m_matCam_inv{m::unit<t_mat_gl>()},
	m_vecCamX{m::create<t_vec_gl>({1.,0.,0.,0.})}, m_vecCamY{m::create<t_vec_gl>({0.,1.,0.,0.})}
{
#if _GL_USE_TIMER != 0
	connect(&m_timer, &QTimer::timeout, this, static_cast<void (GlPlot::*)()>(&GlPlot::tick));
	m_timer.start(std::chrono::milliseconds(1000 / 60));
#endif

	UpdateCam();
	setMouseTracking(true);
}


GlPlot::~GlPlot()
{
#if _GL_USE_TIMER != 0
	m_timer.stop();
#endif
	setMouseTracking(false);
}


qgl_funcs* GlPlot::GetGlFunctions()
{
	qgl_funcs *pGl = nullptr;

	if constexpr(std::is_same_v<qgl_funcs, QOpenGLFunctions>)
		pGl = (qgl_funcs*)context()->functions();
	else
		pGl = (qgl_funcs*)context()->versionFunctions<qgl_funcs>();

	if(!pGl)
		std::cerr << "No suitable GL interface found." << std::endl;

	return pGl;
}


// ----------------------------------------------------------------------------

GlPlotObj GlPlot::CreateTriangleObject(const std::vector<t_vec3_gl>& verts,
	const std::vector<t_vec3_gl>& triagverts, const std::vector<t_vec3_gl>& norms,
	const t_vec_gl& color, bool bUseVertsAsNorm)
{
	qgl_funcs* pGl = GetGlFunctions();
	GLint attrVertex = m_attrVertex;
	GLint attrVertexNormal = m_attrVertexNormal;
	GLint attrVertexColor = m_attrVertexColor;

	GlPlotObj obj;
	obj.m_type = GlPlotObjType::TRIANGLES;
	obj.m_color = color;

	// flatten vertex array into raw float array
	auto to_float_array = [](const std::vector<t_vec3_gl>& verts, int iRepeat=1, int iElems=3, bool bNorm=false)
		-> std::vector<t_real_gl>
	{
		std::vector<t_real_gl> vecRet;
		vecRet.reserve(iRepeat*verts.size()*iElems);

		for(const t_vec3_gl& vert : verts)
		{
			t_real_gl norm = bNorm ? m::norm<t_vec3_gl>(vert) : 1;

			for(int i=0; i<iRepeat; ++i)
				for(int iElem=0; iElem<iElems; ++iElem)
					vecRet.push_back(vert[iElem] / norm);
		}

		return vecRet;
	};

	// main vertex array object
	pGl->glGenVertexArrays(1, &obj.m_vertexarr);
	pGl->glBindVertexArray(obj.m_vertexarr);

	{	// vertices
		obj.m_pvertexbuf = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);

		obj.m_pvertexbuf->create();
		obj.m_pvertexbuf->bind();
		BOOST_SCOPE_EXIT(&obj) { obj.m_pvertexbuf->release(); } BOOST_SCOPE_EXIT_END

		auto vecVerts = to_float_array(triagverts, 1,3, false);
		obj.m_pvertexbuf->allocate(vecVerts.data(), vecVerts.size()*sizeof(typename decltype(vecVerts)::value_type));
		pGl->glVertexAttribPointer(attrVertex, 3, GL_FLOAT, 0, 0, nullptr);
	}

	{	// normals
		obj.m_pnormalsbuf = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);

		obj.m_pnormalsbuf->create();
		obj.m_pnormalsbuf->bind();
		BOOST_SCOPE_EXIT(&obj) { obj.m_pnormalsbuf->release(); } BOOST_SCOPE_EXIT_END

		auto vecNorms = bUseVertsAsNorm ?
			to_float_array(triagverts, 1,3, true) :
			to_float_array(norms, 3,3, false);
		obj.m_pnormalsbuf->allocate(vecNorms.data(), vecNorms.size()*sizeof(typename decltype(vecNorms)::value_type));
		pGl->glVertexAttribPointer(attrVertexNormal, 3, GL_FLOAT, 0, 0, nullptr);
	}

	{	// colors
		obj.m_pcolorbuf = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);

		obj.m_pcolorbuf->create();
		obj.m_pcolorbuf->bind();
		BOOST_SCOPE_EXIT(&obj) { obj.m_pcolorbuf->release(); } BOOST_SCOPE_EXIT_END

		std::vector<t_real_gl> vecCols;
		vecCols.reserve(4*triagverts.size());
		for(std::size_t iVert=0; iVert<triagverts.size(); ++iVert)
		{
			for(int icol=0; icol<obj.m_color.size(); ++icol)
				vecCols.push_back(obj.m_color[icol]);
		}

		obj.m_pcolorbuf->allocate(vecCols.data(), vecCols.size()*sizeof(typename decltype(vecCols)::value_type));
		pGl->glVertexAttribPointer(attrVertexColor, 4, GL_FLOAT, 0, 0, nullptr);
	}


	obj.m_vertices = std::move(verts);
	obj.m_triangles = std::move(triagverts);
	LOGGLERR(pGl)

	return obj;
}


GlPlotObj GlPlot::CreateLineObject(const std::vector<t_vec3_gl>& verts, const t_vec_gl& color)
{
	qgl_funcs* pGl = GetGlFunctions();
	GLint attrVertex = m_attrVertex;
	GLint attrVertexColor = m_attrVertexColor;

	GlPlotObj obj;
	obj.m_type = GlPlotObjType::LINES;
	obj.m_color = color;

	// flatten vertex array into raw float array
	auto to_float_array = [](const std::vector<t_vec3_gl>& verts, int iElems=3) -> std::vector<t_real_gl>
	{
		std::vector<t_real_gl> vecRet;
		vecRet.reserve(verts.size()*iElems);

		for(const t_vec3_gl& vert : verts)
		{
			for(int iElem=0; iElem<iElems; ++iElem)
				vecRet.push_back(vert[iElem]);
		}

		return vecRet;
	};

	// main vertex array object
	pGl->glGenVertexArrays(1, &obj.m_vertexarr);
	pGl->glBindVertexArray(obj.m_vertexarr);

	{	// vertices
		obj.m_pvertexbuf = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);

		obj.m_pvertexbuf->create();
		obj.m_pvertexbuf->bind();
		BOOST_SCOPE_EXIT(&obj) { obj.m_pvertexbuf->release(); } BOOST_SCOPE_EXIT_END

		auto vecVerts = to_float_array(verts, 3);
		obj.m_pvertexbuf->allocate(vecVerts.data(), vecVerts.size()*sizeof(typename decltype(vecVerts)::value_type));
		pGl->glVertexAttribPointer(attrVertex, 3, GL_FLOAT, 0, 0, nullptr);
	}

	{	// colors
		obj.m_pcolorbuf = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);

		obj.m_pcolorbuf->create();
		obj.m_pcolorbuf->bind();
		BOOST_SCOPE_EXIT(&obj) { obj.m_pcolorbuf->release(); } BOOST_SCOPE_EXIT_END

		std::vector<t_real_gl> vecCols;
		vecCols.reserve(4*verts.size());
		for(std::size_t iVert=0; iVert<verts.size(); ++iVert)
		{
			for(int icol=0; icol<obj.m_color.size(); ++icol)
				vecCols.push_back(obj.m_color[icol]);
		}

		obj.m_pcolorbuf->allocate(vecCols.data(), vecCols.size()*sizeof(typename decltype(vecCols)::value_type));
		pGl->glVertexAttribPointer(attrVertexColor, 4, GL_FLOAT, 0, 0, nullptr);
	}


	obj.m_vertices = std::move(verts);
	LOGGLERR(pGl)

	return obj;
}


t_mat_gl GlPlot::GetArrowMatrix(const t_vec_gl& vecTo, t_real_gl scale, const t_vec_gl& vecTrans, const t_vec_gl& vecFrom)
{
	t_mat_gl mat = m::unit<t_mat_gl>(4);
	mat *= m::rotation<t_mat_gl, t_vec_gl>(vecFrom, vecTo);
	mat *= m::hom_scaling<t_mat_gl>(scale, scale, scale);
	mat *= m::hom_translation<t_mat_gl>(vecTrans[0], vecTrans[1], vecTrans[2]);
	return mat;
}


void GlPlot::SetObjectMatrix(std::size_t idx, const t_mat_gl& mat)
{
	if(idx >= m_objs.size()) return;
	m_objs[idx].m_mat = mat;
}

void GlPlot::SetObjectLabel(std::size_t idx, const std::string& label)
{
	if(idx >= m_objs.size()) return;
	m_objs[idx].m_label = label;
}

void GlPlot::SetObjectVisible(std::size_t idx, bool visible)
{
	if(idx >= m_objs.size()) return;
	m_objs[idx].m_visible = visible;
}


std::size_t GlPlot::AddSphere(t_real_gl rad, t_real_gl x, t_real_gl y, t_real_gl z,
	t_real_gl r, t_real_gl g, t_real_gl b, t_real_gl a)
{
	auto solid = m::create_icosahedron<t_vec3_gl>(1);
	auto [triagverts, norms, uvs] = m::spherify<t_vec3_gl>(
		m::subdivide_triangles<t_vec3_gl>(
			m::create_triangles<t_vec3_gl>(solid), 2), rad);

	auto obj = CreateTriangleObject(std::get<0>(solid), triagverts, norms, m::create<t_vec_gl>({r,g,b,a}), true);
	obj.m_mat = m::hom_translation<t_mat_gl>(x, y, z);
	m_objs.emplace_back(std::move(obj));

	return m_objs.size()-1;		// object handle
}


std::size_t GlPlot::AddCylinder(t_real_gl rad, t_real_gl h, t_real_gl x, t_real_gl y, t_real_gl z,
	t_real_gl r, t_real_gl g, t_real_gl b, t_real_gl a)
{
	auto solid = m::create_cylinder<t_vec3_gl>(rad, h, true);
	auto [triagverts, norms, uvs] = m::create_triangles<t_vec3_gl>(solid);

	auto obj = CreateTriangleObject(std::get<0>(solid), triagverts, norms, m::create<t_vec_gl>({r,g,b,a}), false);
	obj.m_mat = m::hom_translation<t_mat_gl>(x, y, z);
	m_objs.emplace_back(std::move(obj));

	return m_objs.size()-1;		// object handle
}


std::size_t GlPlot::AddCone(t_real_gl rad, t_real_gl h, t_real_gl x, t_real_gl y, t_real_gl z,
	t_real_gl r, t_real_gl g, t_real_gl b, t_real_gl a)
{
	auto solid = m::create_cone<t_vec3_gl>(rad, h);
	auto [triagverts, norms, uvs] = m::create_triangles<t_vec3_gl>(solid);

	auto obj = CreateTriangleObject(std::get<0>(solid), triagverts, norms, m::create<t_vec_gl>({r,g,b,a}), false);
	obj.m_mat = m::hom_translation<t_mat_gl>(x, y, z);
	m_objs.emplace_back(std::move(obj));

	return m_objs.size()-1;		// object handle
}


std::size_t GlPlot::AddArrow(t_real_gl rad, t_real_gl h, t_real_gl x, t_real_gl y, t_real_gl z,
	t_real_gl r, t_real_gl g, t_real_gl b, t_real_gl a)
{
	auto solid = m::create_cylinder<t_vec3_gl>(rad, h, 2, 32, rad, rad*1.5);
	auto [triagverts, norms, uvs] = m::create_triangles<t_vec3_gl>(solid);

	auto obj = CreateTriangleObject(std::get<0>(solid), triagverts, norms, m::create<t_vec_gl>({r,g,b,a}), false);
	obj.m_mat = GetArrowMatrix(m::create<t_vec_gl>({1,0,0}), 1., m::create<t_vec_gl>({x,y,z}), m::create<t_vec_gl>({0,0,1}));
	obj.m_labelPos = m::create<t_vec3_gl>({0., 0., 0.75});
	m_objs.emplace_back(std::move(obj));

	return m_objs.size()-1;		// object handle
}


std::size_t GlPlot::AddCoordinateCross(t_real_gl min, t_real_gl max)
{
	auto col = m::create<t_vec_gl>({0,0,0,1});
	auto verts = std::vector<t_vec3_gl>
	{{
		m::create<t_vec3_gl>({min,0,0}), m::create<t_vec3_gl>({max,0,0}),
		m::create<t_vec3_gl>({0,min,0}), m::create<t_vec3_gl>({0,max,0}),
		m::create<t_vec3_gl>({0,0,min}), m::create<t_vec3_gl>({0,0,max}),
	}};

	auto obj = CreateLineObject(verts, col);
	m_objs.emplace_back(std::move(obj));

	return m_objs.size()-1;		// object handle
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
void GlPlot::initializeGL()
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

		in vec4 vertex;
		in vec4 normal;
		in vec4 vertexcolor;
		out vec4 fragcolor;

		uniform mat4 proj = mat4(1.);
		uniform mat4 cam = mat4(1.);
		uniform mat4 obj = mat4(1.);

		//vec4 vertexcolor = vec4(0, 0, 1, 1);
		vec3 light_dir = vec3(2, 2, -1);

		float lighting(vec3 lightdir)
		{
			float I = dot(vec3(cam*normal), normalize(lightdir));
			if(I < 0) I = 0;
			return I;
		}

		void main()
		{
			gl_Position = proj * cam * obj * vertex;

			float I = lighting(light_dir);
			fragcolor = vertexcolor * I;
			fragcolor[3] = 1;
		})RAW";
	// --------------------------------------------------------------------


	// set glsl version
	std::string strGlsl = std::to_string(_GL_MAJ_VER*100 + _GL_MIN_VER*10);
	for(std::string* strSrc : { &strFragShader, &strVertexShader })
		algo::replace_all(*strSrc, std::string("${GLSL_VERSION}"), strGlsl);


	// GL functions
	auto *pGl = GetGlFunctions();
	if(!pGl) return;

	std::ostringstream ostrGlDescr;
	ostrGlDescr << (char*)pGl->glGetString(GL_VERSION) << ", "
		<< (char*)pGl->glGetString(GL_VENDOR) << ", "
		<< (char*)pGl->glGetString(GL_RENDERER) << ", "
		<< "glsl: " << (char*)pGl->glGetString(GL_SHADING_LANGUAGE_VERSION);
	m_strGlDescr = ostrGlDescr.str();
	//std::cerr << __func__ << ": " << m_strGlDescr << std::endl;
	LOGGLERR(pGl);


	// shaders
	{
		static QMutex shadermutex;
		shadermutex.lock();
		BOOST_SCOPE_EXIT(&shadermutex) { shadermutex.unlock(); } BOOST_SCOPE_EXIT_END

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

		if(!m_pShaders->link())
			shader_err("Cannot link shaders.");

		m_uniMatrixCam = m_pShaders->uniformLocation("cam");
		m_uniMatrixProj = m_pShaders->uniformLocation("proj");
		m_uniMatrixObj = m_pShaders->uniformLocation("obj");
		m_attrVertex = m_pShaders->attributeLocation("vertex");
		m_attrVertexNormal = m_pShaders->attributeLocation("normal");
		m_attrVertexColor = m_pShaders->attributeLocation("vertexcolor");
	}
	LOGGLERR(pGl);


	// 3d objects
	{
		AddCoordinateCross(-2.5, 2.5);

		// test objects
		//AddArrow(0.05, 1., 0.,0.,0.5,  0.,0.,0.75,1.);
		//AddCone(1., 1., 0.,0.,0.,  0.,0.5,0.,1.);
		//AddSphere(0.2, 0.,0.,2., 0.,0.,1.,1.);
		//AddCylinder(0.2, 0.5, 0.,0.,-2., 0.,0.,1.,1.);
	}


	// options
	pGl->glCullFace(GL_BACK);
	pGl->glEnable(GL_CULL_FACE);

	pGl->glEnable(GL_MULTISAMPLE | GL_LINE_SMOOTH | GL_POLYGON_SMOOTH);
	pGl->glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	pGl->glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	emit AfterGLInitialisation();
}


void GlPlot::resizeGL(int w, int h)
{
	m_iScreenDims[0] = w;
	m_iScreenDims[1] = h;

	auto *pContext = context();
	if(!pContext) return;

	//std::cerr << std::dec << __func__ << ": w = " << w << ", h = " << h << std::endl;
	m_matViewport = m::hom_viewport<t_mat_gl>(w, h, 0., 1.);
	std::tie(m_matViewport_inv, std::ignore) = m::inv<t_mat_gl>(m_matViewport);

	auto *pGl = GetGlFunctions();
	pGl->glViewport(0, 0, w, h);
	pGl->glDepthRange(0, 1);

	m_matPerspective = m::hom_perspective<t_mat_gl>(0.01, 100., m::pi<t_real_gl>*0.5, t_real_gl(h)/t_real_gl(w));
	std::tie(m_matPerspective_inv, std::ignore) = m::inv<t_mat_gl>(m_matPerspective);


	// bind shaders
	m_pShaders->bind();
	BOOST_SCOPE_EXIT(m_pShaders) { m_pShaders->release(); } BOOST_SCOPE_EXIT_END
	LOGGLERR(pGl);

	// set matrices
	m_pShaders->setUniformValue(m_uniMatrixCam, m_matCam);
	m_pShaders->setUniformValue(m_uniMatrixProj, m_matPerspective);
	LOGGLERR(pGl);

}


void GlPlot::paintGL()
{
	auto *pContext = context();
	if(!pContext) return;
	QPainter painter(this);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);


	// gl painting
	{
		auto *pPlot = this;
		BOOST_SCOPE_EXIT(pPlot, &painter)
		{
// if the frame is not already updated by the timer, directly update it
#if _GL_USE_TIMER == 0
			QMetaObject::invokeMethod(pPlot, static_cast<void (QOpenGLWidget::*)()>(&QOpenGLWidget::update),
				Qt::ConnectionType::QueuedConnection);
#endif
			painter.endNativePainting();
		}
		BOOST_SCOPE_EXIT_END

		if(m_bPickerNeedsUpdate)
			UpdatePicker();


		auto *pGl = GetGlFunctions();
		painter.beginNativePainting();


		// clear
		pGl->glClearColor(1., 1., 1., 1.);
		pGl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		pGl->glEnable(GL_DEPTH_TEST);


		// bind shaders
		m_pShaders->bind();
		BOOST_SCOPE_EXIT(m_pShaders) { m_pShaders->release(); } BOOST_SCOPE_EXIT_END
		LOGGLERR(pGl);


		// set cam matrix
		m_pShaders->setUniformValue(m_uniMatrixCam, m_matCam);

		// render objects
		for(auto& obj : m_objs)
		{
			if(!obj.m_visible) continue;

			// main vertex array object
			pGl->glBindVertexArray(obj.m_vertexarr);

			m_pShaders->setUniformValue(m_uniMatrixObj, obj.m_mat);

			pGl->glEnableVertexAttribArray(m_attrVertex);
			if(obj.m_type == GlPlotObjType::TRIANGLES)
				pGl->glEnableVertexAttribArray(m_attrVertexNormal);
			pGl->glEnableVertexAttribArray(m_attrVertexColor);
			BOOST_SCOPE_EXIT(pGl, &m_attrVertex, &m_attrVertexNormal, &m_attrVertexColor)
			{
				pGl->glDisableVertexAttribArray(m_attrVertexColor);
				pGl->glDisableVertexAttribArray(m_attrVertexNormal);
				pGl->glDisableVertexAttribArray(m_attrVertex);
			}
			BOOST_SCOPE_EXIT_END
			LOGGLERR(pGl);

			if(obj.m_type == GlPlotObjType::TRIANGLES)
				pGl->glDrawArrays(GL_TRIANGLES, 0, obj.m_triangles.size());
			else if(obj.m_type == GlPlotObjType::LINES)
				pGl->glDrawArrays(GL_LINES, 0, obj.m_vertices.size());
			else
				std::cerr << "Error: Unknown plot object." << std::endl;

			LOGGLERR(pGl);
		}

		pGl->glDisable(GL_DEPTH_TEST);
	}


	// qt painting
	{
		// coordinate labels
		painter.drawText(GlToScreenCoords(m::create<t_vec_gl>({0.,0.,0.,1.})), "0");
		for(t_real_gl f=-2.; f<=2.; f+=0.5)
		{
			if(m::equals<t_real_gl>(f, 0))
				continue;

			std::ostringstream ostrF;
			ostrF << f;
			painter.drawText(GlToScreenCoords(m::create<t_vec_gl>({f,0.,0.,1.})), ostrF.str().c_str());
			painter.drawText(GlToScreenCoords(m::create<t_vec_gl>({0.,f,0.,1.})), ostrF.str().c_str());
			painter.drawText(GlToScreenCoords(m::create<t_vec_gl>({0.,0.,f,1.})), ostrF.str().c_str());
		}

		painter.drawText(GlToScreenCoords(m::create<t_vec_gl>({3.,0.,0.,1.})), "x");
		painter.drawText(GlToScreenCoords(m::create<t_vec_gl>({0.,3.,0.,1.})), "y");
		painter.drawText(GlToScreenCoords(m::create<t_vec_gl>({0.,0.,3.,1.})), "z");


		// render object labels
		for(auto& obj : m_objs)
		{
			if(!obj.m_visible) continue;

			if(obj.m_label != "")
			{
				t_vec3_gl posLabel3d = obj.m_mat * obj.m_labelPos;
				auto posLabel2d = GlToScreenCoords(m::create<t_vec_gl>({posLabel3d[0], posLabel3d[1], posLabel3d[2], 1.}));

				QFont fontOrig = painter.font();
				QPen penOrig = painter.pen();
				QFont fontLabel = fontOrig;
				QPen penLabel = penOrig;

				fontLabel.setWeight(QFont::Medium);
				painter.setFont(fontLabel);
				painter.drawText(posLabel2d, obj.m_label.c_str());

				fontLabel.setWeight(QFont::Normal);
				penLabel.setColor(QColor(int(obj.m_color[0]*255.), int(obj.m_color[1]*255.), int(obj.m_color[2]*255.), int(obj.m_color[3]*255.)));
				painter.setFont(fontLabel);
				painter.setPen(penLabel);
				painter.drawText(posLabel2d, obj.m_label.c_str());

				// restore original styles
				painter.setFont(fontOrig);
				painter.setPen(penOrig);
			}
		}
	}
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
void GlPlot::tick()
{
	tick(std::chrono::milliseconds(1000 / 60));
}

void GlPlot::tick(const std::chrono::milliseconds& ms)
{
	// TODO
	UpdateCam();
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
void GlPlot::UpdateCam()
{
	// zoom
	t_mat_gl matZoom = m::unit<t_mat_gl>();
	matZoom(0,0) = matZoom(1,1) = matZoom(2,2) = m_zoom;

	m_matCam = m_matCamBase;
	m_matCam *= m_matCamRot;
	m_matCam *= matZoom;
	std::tie(m_matCam_inv, std::ignore) = m::inv<t_mat_gl>(m_matCam);

	m_bPickerNeedsUpdate = true;
	QMetaObject::invokeMethod(this, static_cast<void (QOpenGLWidget::*)()>(&QOpenGLWidget::update),
		Qt::ConnectionType::QueuedConnection);
}


QPointF GlPlot::GlToScreenCoords(const t_vec_gl& vec4, bool *pVisible)
{
	auto [ vecPersp, vec ] =
		m::hom_to_screen_coords<t_mat_gl, t_vec_gl>(vec4, m_matCam, m_matPerspective, m_matViewport, true);

	// position not visible -> return a point outside the viewport
	if(vecPersp[2] > 1.)
	{
		if(pVisible) *pVisible = false;
		return QPointF(-1*m_iScreenDims[0], -1*m_iScreenDims[1]);
	}

	if(pVisible) *pVisible = true;
	return QPointF(vec[0], vec[1]);
}


void GlPlot::zoom(t_real_gl val)
{
	m_zoom *= std::pow(2., val/64.);
	UpdateCam();
}

void GlPlot::ResetZoom()
{
	m_zoom = 1;
	UpdateCam();
}


void GlPlot::BeginRotation()
{
	if(!m_bInRotation)
	{
		m_posMouseRotationStart = m_posMouse;
		m_bInRotation = true;
	}
}

void GlPlot::EndRotation()
{
	if(m_bInRotation)
	{
		auto diff = (m_posMouse - m_posMouseRotationStart);
		m_phi_saved += diff.x();
		m_theta_saved += diff.y();

		m_bInRotation = false;
	}
}


void GlPlot::UpdatePicker()
{
	const bool show_picked_triangle = false;

	// picker ray
	auto [org, dir] = m::hom_line_from_screen_coords<t_mat_gl, t_vec_gl>(
		m_posMouse.x(), m_posMouse.y(), 0., 1., m_matCam_inv,
		m_matPerspective_inv, m_matViewport_inv, &m_matViewport, true);
	t_vec3_gl org3 = m::create<t_vec3_gl>({org[0], org[1], org[2]});
	t_vec3_gl dir3 = m::create<t_vec3_gl>({dir[0], dir[1], dir[2]});


	// intersection with unit sphere around origin
	bool hasSphereInters = false;
	t_vec_gl vecClosestSphereInters = m::create<t_vec_gl>({0,0,0,0});

	auto intersUnitSphere =
		m::intersect_line_sphere<t_vec3_gl, std::vector>(org3, dir3,
			m::create<t_vec3_gl>({0,0,0}), t_real_gl(1));
	for(const auto& result : intersUnitSphere)
	{
		t_vec_gl vecInters4 = m::create<t_vec_gl>({result[0], result[1], result[2], 1});

		if(!hasSphereInters)
		{	// first intersection
			vecClosestSphereInters = vecInters4;
			hasSphereInters = true;
		}
		else
		{	// test if next intersection is closer...
			t_vec_gl oldPosTrafo = m_matCam * vecClosestSphereInters;
			t_vec_gl newPosTrafo = m_matCam * vecInters4;

			// ... it is closer.
			if(m::norm(newPosTrafo) < m::norm(oldPosTrafo))
				vecClosestSphereInters = vecInters4;
		}
	}


	// intersection with geometry
	// 3 vertices with rgba colour
	const t_real_gl colSelected[] = {1.,1.,1.,1., 1.,1.,1.,1., 1.,1.,1.,1.};

	bool hasInters = false;
	t_vec_gl vecClosestInters = m::create<t_vec_gl>({0,0,0,0});
	std::size_t objInters = 0xffffffff;

	for(std::size_t curObj=0; curObj<m_objs.size(); ++curObj)
	{
		auto& obj = m_objs[curObj];

		if(obj.m_type != GlPlotObjType::TRIANGLES || !obj.m_visible)
			continue;

		//obj.m_pickerInters.clear();

		t_real_gl objcol[4*3];
		if(show_picked_triangle)
		{
			// TODO: only works if qvector4d stores its data linearly!
			std::copy(&obj.m_color[0], &obj.m_color[0]+4, objcol+0*4);
			std::copy(&obj.m_color[0], &obj.m_color[0]+4, objcol+1*4);
			std::copy(&obj.m_color[0], &obj.m_color[0]+4, objcol+2*4);
		}

		obj.m_pcolorbuf->bind();
		BOOST_SCOPE_EXIT(&obj) { obj.m_pcolorbuf->release(); } BOOST_SCOPE_EXIT_END

		for(std::size_t startidx=0; startidx+2<obj.m_triangles.size(); startidx+=3)
		{
			std::vector<t_vec3_gl> poly{{ obj.m_triangles[startidx+0], obj.m_triangles[startidx+1], obj.m_triangles[startidx+2] }};
			auto [vecInters, bInters, lamInters] =
				m::intersect_line_poly<t_vec3_gl, t_mat_gl>(org3, dir3, poly, obj.m_mat);

			if(bInters)
			{
				t_vec_gl vecInters4 = m::create<t_vec_gl>({vecInters[0], vecInters[1], vecInters[2], 1});

				if(!hasInters)
				{	// first intersection
					vecClosestInters = vecInters4;
					objInters = curObj;
					hasInters = true;
				}
				else
				{	// test if next intersection is closer...
					t_vec_gl oldPosTrafo = m_matCam * vecClosestInters;
					t_vec_gl newPosTrafo = m_matCam * vecInters4;

					if(m::norm(newPosTrafo) < m::norm(oldPosTrafo))
					{	// ...it is closer
						vecClosestInters = vecInters4;
						objInters = curObj;
					}
				}

				//obj.m_pickerInters.emplace_back(std::move(vecInters));
			}

			if(show_picked_triangle)
			{
				if(bInters)
					obj.m_pcolorbuf->write(sizeof(colSelected[0])*startidx*4, colSelected, sizeof(colSelected));
				else
					obj.m_pcolorbuf->write(sizeof(objcol[0])*startidx*4, objcol, sizeof(objcol));
			}
		}
	}

	m_bPickerNeedsUpdate = false;
	t_vec3_gl vecClosestInters3 = m::create<t_vec3_gl>({vecClosestInters[0], vecClosestInters[1], vecClosestInters[2]});
	t_vec3_gl vecClosestSphereInters3 = m::create<t_vec3_gl>({vecClosestSphereInters[0], vecClosestSphereInters[1], vecClosestSphereInters[2]});
	emit PickerIntersection(hasInters ? &vecClosestInters3 : nullptr, objInters,
		hasSphereInters ? &vecClosestSphereInters3 : nullptr);
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
void GlPlot::mousePressEvent(QMouseEvent *pEvt)
{
	bool bLeftDown = 0, bRightDown = 0, bMidDown = 0;

	if(pEvt->buttons() & Qt::LeftButton) bLeftDown = 1;
	if(pEvt->buttons() & Qt::RightButton) bRightDown = 1;
	if(pEvt->buttons() & Qt::MiddleButton) bMidDown = 1;

	if(bMidDown)
		ResetZoom();
	if(bRightDown)
		BeginRotation();

	pEvt->accept();
}

void GlPlot::mouseReleaseEvent(QMouseEvent *pEvt)
{
	bool bLeftUp = 0, bRightUp = 0, bMidUp = 0;

	if((pEvt->buttons() & Qt::LeftButton) == 0) bLeftUp = 1;
	if((pEvt->buttons() & Qt::RightButton) == 0) bRightUp = 1;
	if((pEvt->buttons() & Qt::MiddleButton) == 0) bMidUp = 1;

	if(bRightUp)
		EndRotation();

	pEvt->accept();
}

void GlPlot::mouseMoveEvent(QMouseEvent *pEvt)
{
	QPointF pos = pEvt->localPos();
	pEvt->accept();

	m_posMouse = pos;

	if(m_bInRotation)
	{
		auto diff = (m_posMouse - m_posMouseRotationStart);
		t_real_gl phi = diff.x() + m_phi_saved;
		t_real_gl theta = diff.y() + m_theta_saved;

		m_matCamRot = m::rotation<t_mat_gl, t_vec_gl>(m_vecCamX, theta/180.*M_PI, 0);
		m_matCamRot *= m::rotation<t_mat_gl, t_vec_gl>(m_vecCamY, phi/180.*M_PI, 0);
	}

	UpdateCam();
}

void GlPlot::wheelEvent(QWheelEvent *pEvt)
{
	const t_real_gl degrees = pEvt->angleDelta().y() / 8.;
	zoom(degrees);

	pEvt->accept();
}
// ----------------------------------------------------------------------------
