/**
 * vk window
 * @author Tobias Weber
 * @date Feb-2021
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *  * https://code.qt.io/cgit/qt/qtbase.git/tree/examples/vulkan/shared/trianglerenderer.cpp
 *  * https://doc.qt.io/qt-5/qvulkanwindow.html
 *  * https://doc.qt.io/qt-5/qvulkaninstance.html
 *  * https://doc.qt.io/qt-5/qvulkanwindowrenderer.html
 *  * https://doc.qt.io/qt-5/qtgui-hellovulkanwindow-example.html
 */

#include "wnd.h"

#include <QApplication>
#include <QMainWindow>
#include <QLoggingCategory>

#include <locale>
#include <iostream>
#include <random>

#include <boost/scope_exit.hpp>


// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

template<class t_num>
t_num get_rand(t_num min=1, t_num max=-1)
{
	static std::mt19937 rng{std::random_device{}()};

	if(max <= min)
	{
		min = std::numeric_limits<t_num>::lowest() / 10.;
		max = std::numeric_limits<t_num>::max() / 10.;
	}

	if constexpr(std::is_integral_v<t_num>)
		return std::uniform_int_distribution<t_num>(min, max)(rng);
	else
		return std::uniform_real_distribution<t_num>(min, max)(rng);
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// vk window
// ----------------------------------------------------------------------------

VkWnd::VkWnd(std::shared_ptr<QVulkanInstance>& vk, QWindow* parent)
	: QVulkanWindow{parent}, m_vkinst{vk}
{
	setVulkanInstance(m_vkinst.get());

	std::chrono::milliseconds ticks{1000 / 60};
	connect(&m_timer, &QTimer::timeout,
		[this, ticks]() -> void
		{
			if(!m_vkrenderer)
				return;

			m_vkrenderer->tick(ticks);
			m_runningtime += ticks;

			// pendulum 1 using closed-form solution
			{
				// pendulum mass
				t_real t = t_real(m_runningtime.count()) / t_real(1000);
				t_real phi = m_pendulum.GetPhiAbs(t);
				t_vec pos = m_pendulum.GetPos(phi);
				std::swap(pos[1], pos[2]);
				if(PolyObject *sphere = m_vkrenderer->GetObject(m_sphere_indices[0]); sphere)
					sphere->SetMatrix(m::hom_translation<t_mat, t_real>(pos[0], pos[1], pos[2]));

				// pendulum thread
				t_mat mat = m::unit<t_mat>(4);
				mat *= m::hom_translation<t_mat>(0.f, m_pendulum.GetLength(), 0.f);
				mat *= m::rotation<t_mat, t_vec>(m::create<t_vec>({0,0,1}), phi);
				mat *= m::hom_translation<t_mat>(0.f, -m_pendulum.GetLength()*0.5f, 0.f);
				mat *= m::rotation<t_mat, t_vec>(m::create<t_vec>({1,0,0}), m::pi<t_real>*0.5f);
				if(PolyObject *cyl = m_vkrenderer->GetObject(m_cyl_indices[0]); cyl)
					cyl->SetMatrix(mat);
			}

			// pendulum 2 using numerical solution of differential equation
			{
				// pendulum mass
				t_real dt = t_real(ticks.count()) / t_real(1000);
				t_real phi = m_pendulum.StepPhiEuler(dt);
				t_vec pos = m_pendulum.GetPos(phi);
				std::swap(pos[1], pos[2]);
				if(PolyObject *sphere = m_vkrenderer->GetObject(m_sphere_indices[1]); sphere)
					sphere->SetMatrix(m::hom_translation<t_mat, t_real>(pos[0], pos[1], pos[2]+2.f));

				// pendulum thread
				t_mat mat = m::unit<t_mat>(4);
				mat *= m::hom_translation<t_mat>(0.f, m_pendulum.GetLength(), 2.f);
				mat *= m::rotation<t_mat, t_vec>(m::create<t_vec>({0,0,1}), phi);
				mat *= m::hom_translation<t_mat>(0.f, -m_pendulum.GetLength()*0.5f, 0.f);
				mat *= m::rotation<t_mat, t_vec>(m::create<t_vec>({1,0,0}), m::pi<t_real>*0.5f);
				if(PolyObject *cyl = m_vkrenderer->GetObject(m_cyl_indices[1]); cyl)
					cyl->SetMatrix(mat);
			}

			t_vec pos_cam = m_vkrenderer->GetCamera().GetPosition();

			auto status = QString{"Running time: %1 s, camera: %2, %3, %4"}
				.arg(m_runningtime.count()/1000, 0)
				.arg(pos_cam[0], 0, 'f', 1)
				.arg(pos_cam[1], 0, 'f', 1)
				.arg(pos_cam[2], 0, 'f', 1);
			emit EmitStatusMsg(status);
		});

	m_timer.start(ticks);
}


VkWnd::~VkWnd()
{
	m_timer.stop();
}


QVulkanWindowRenderer* VkWnd::createRenderer()
{
	if(m_vkrenderer)
		delete m_vkrenderer;

	m_vkrenderer = new VkRenderer(m_vkinst, this);

	CreateObjects();
	return m_vkrenderer;
}


void VkWnd::CreateObjects()
{
	// ------------------------------------------------------------------------
	// add objects
	// ------------------------------------------------------------------------
	VkRenderer *renderer = GetRenderer();

	// planes
	t_real plane_size = 10.;

	PolyObject plane;
	plane.CreatePlaneGeometry(
		m::hom_translation<t_mat, t_real>(0, -2, 0),
		m::create<t_vec3>({0, -1, 0}), plane_size, 0.75, 0.75, 0.75);
	renderer->AddObject(plane);

	// sphere for pendulum 1
	{
		t_real rad = 0.5;
		t_real col = get_rand<t_real>(0.5, 1.);

		PolyObject sphere;
		sphere.CreateSphereGeometry(
			m::hom_translation<t_mat, t_real>(0, 0, 0),
			rad, 0., 0., col);
		m_sphere_indices[0] = renderer->AddObject(sphere);
	}

	// cylinder for pendulum 1
	{
		t_real rad = 0.1;
		t_real height = m_pendulum.GetLength();
		t_real col = get_rand<t_real>(0.5, 1.);

		PolyObject cyl;
		cyl.CreateCylinderGeometry(
			m::hom_translation<t_mat, t_real>(0, 0, 0),
			rad, height, 0., col, 0.);
		m_cyl_indices[0] = renderer->AddObject(cyl);
	}

	// sphere for pendulum 2
	{
		t_real rad = 0.5;
		t_real col = get_rand<t_real>(0.5, 1.);

		PolyObject sphere;
		sphere.CreateSphereGeometry(
			m::hom_translation<t_mat, t_real>(0, 0, 0),
			rad, 0., 0., col);
		m_sphere_indices[1] = renderer->AddObject(sphere);
	}

	// cylinder for pendulum 2
	{
		t_real rad = 0.1;
		t_real height = m_pendulum.GetLength();
		t_real col = get_rand<t_real>(0.5, 1.);

		PolyObject cyl;
		cyl.CreateCylinderGeometry(
			m::hom_translation<t_mat, t_real>(0, 0, 0),
			rad, height, 0., col, 0.);
		m_cyl_indices[1] = renderer->AddObject(cyl);
	}
	// ------------------------------------------------------------------------
}


void VkWnd::mouseMoveEvent(QMouseEvent *pEvt)
{
	if(m_vkrenderer)
		m_vkrenderer->SetMousePos(pEvt->localPos());

	QVulkanWindow::mouseMoveEvent(pEvt);
}


void VkWnd::keyPressEvent(QKeyEvent *pEvt)
{
	if(!m_vkrenderer)
		return;

	if(pEvt->key() == Qt::Key_Space)
		m_vkrenderer->TogglePerspective();

	if(pEvt->key() == Qt::Key_A)
		m_vkrenderer->SetMoving(0, 1.);
	if(pEvt->key() == Qt::Key_D)
		m_vkrenderer->SetMoving(0, -1.);
	if(pEvt->key() == Qt::Key_W)
		m_vkrenderer->SetMoving(2, 1.);
	if(pEvt->key() == Qt::Key_S)
		m_vkrenderer->SetMoving(2, -1.);
	if(pEvt->key() == Qt::Key_E)
		m_vkrenderer->SetMoving(1, 1.);
	if(pEvt->key() == Qt::Key_Q)
		m_vkrenderer->SetMoving(1, -1.);

	if(pEvt->key() == Qt::Key_Up /*|| pEvt->key() == Qt::Key_I*/)
		m_vkrenderer->SetRotating(0, 1.);
	if(pEvt->key() == Qt::Key_Down /*|| pEvt->key() == Qt::Key_K*/)
		m_vkrenderer->SetRotating(0, -1.);
	if(pEvt->key() == Qt::Key_Left /*|| pEvt->key() == Qt::Key_J*/)
		m_vkrenderer->SetRotating(1, -1.);
	if(pEvt->key() == Qt::Key_Right /*|| pEvt->key() == Qt::Key_L*/)
		m_vkrenderer->SetRotating(1, 1.);
	if(pEvt->key() == Qt::Key_Y)
		m_vkrenderer->SetRotating(2, -1.);
	if(pEvt->key() == Qt::Key_C)
		m_vkrenderer->SetRotating(2, 1.);

	QVulkanWindow::keyPressEvent(pEvt);
}


void VkWnd::keyReleaseEvent(QKeyEvent *pEvt)
{
	if(!m_vkrenderer)
		return;

	if(pEvt->key() == Qt::Key_A || pEvt->key() == Qt::Key_D)
		m_vkrenderer->SetMoving(0, 0.);
	if(pEvt->key() == Qt::Key_W || pEvt->key() == Qt::Key_S)
		m_vkrenderer->SetMoving(2, 0.);
	if(pEvt->key() == Qt::Key_E || pEvt->key() == Qt::Key_Q)
		m_vkrenderer->SetMoving(1, 0.);

	if(pEvt->key() == Qt::Key_Up || pEvt->key() == Qt::Key_Down)
		m_vkrenderer->SetRotating(0, 0.);
	if(pEvt->key() == Qt::Key_Left || pEvt->key() == Qt::Key_Right)
		m_vkrenderer->SetRotating(1, 0.);
	if(pEvt->key() == Qt::Key_Y || pEvt->key() == Qt::Key_C)
		m_vkrenderer->SetRotating(2, 0.);

	QVulkanWindow::keyReleaseEvent(pEvt);
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// main window
// ----------------------------------------------------------------------------
Wnd::Wnd(VkWnd *vkwnd, QWidget* parent)
	: QMainWindow(parent), m_vkwnd{vkwnd}
{
	// set the vk window as central widget
	m_vkwidget = QWidget::createWindowContainer(m_vkwnd);
	m_vkwidget->setFocusPolicy(Qt::StrongFocus);
	//setFocusProxy(m_vkwidget);
	setCentralWidget(m_vkwidget);

	m_statusbar = new QStatusBar(this);
	m_statuslabel = new QLabel(m_statusbar);
	m_statusbar->addPermanentWidget(m_statuslabel, 0);
	setStatusBar(m_statusbar);

	connect(m_vkwnd, &VkWnd::EmitStatusMsg,
		[this](const QString& str)->void
		{
			if(!m_statuslabel)
				return;

			m_statuslabel->setText(str);
		});
}


Wnd::~Wnd()
{
}


void Wnd::resizeEvent(QResizeEvent *evt)
{
	QMainWindow::resizeEvent(evt);
	//m_vkwidget->setFocus();
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// init
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
	try
	{
		// ------------------------------------------------------------------------
		// misc initialisation
		// ------------------------------------------------------------------------
		QLoggingCategory::setFilterRules("*=false\n*.debug=false\n");
		qInstallMessageHandler([](QtMsgType ty, const QMessageLogContext& ctx, const QString& log) -> void
		{
			auto get_msg_type = [](const QtMsgType& _ty) -> std::string
			{
				switch(_ty)
				{
					case QtDebugMsg: return "debug";
					case QtWarningMsg: return "warning";
					case QtCriticalMsg: return "critical";
					case QtFatalMsg: return "fatal";
					case QtInfoMsg: return "info";
					default: return "<unknown>";
				}
			};

			auto get_str = [](const char* pc) -> std::string
			{
				if(!pc) return "<unknown>";
				return std::string{"\""} + std::string{pc} + std::string{"\""};
			};

			std::cerr << "qt " << get_msg_type(ty);
			if(ctx.function)
			{
				std::cerr << " in "
					<< "file " << get_str(ctx.file) << ", "
					<< "function " << get_str(ctx.function) << ", "
					<< "line " << ctx.line;
			}
			std::cerr << ": " << log.toStdString() << std::endl;
		});

		auto app = std::make_unique<QApplication>(argc, argv);
		set_locales();
		// ------------------------------------------------------------------------


		// ------------------------------------------------------------------------
		// vk
		// ------------------------------------------------------------------------
		// create vk instance
		auto vk = std::make_shared<QVulkanInstance>();

		QByteArrayList layers{{
			"VK_LAYER_KHRONOS_validation",
			"VK_EXT_debug_report",
			"VK_EXT_debug_utils",
		}};
		vk->setLayers(layers);
		vk->setFlags(vk->flags() & ~QVulkanInstance::NoDebugOutputRedirect);

		if(!vk->create() || !vk->isValid())
		{
			std::cerr << "Cannot create a valid Vk instance." << std::endl;
			return -1;
		}

		BOOST_SCOPE_EXIT(&vk)
		{
			vk->destroy();
		}
		BOOST_SCOPE_EXIT_END


		// get version infos
		std::string vkver = vk->apiVersion().toString().toStdString();
		if(vkver != "")
			std::cout << "Vk API version: " << vkver << "." << std::endl;

		// get layer infos
		QVulkanInfoVector<QVulkanLayer> vklayers = vk->supportedLayers();
		for(const auto& vklayer : vklayers)
		{
			std::cout << "Vk layer: " << vklayer.name.toStdString()
				<< ", description: " << vklayer.description.toStdString()
				<< ", layer version: " << vklayer.version
				<< ", Vk version: " << vklayer.specVersion.toString().toStdString()
				<< "." << std::endl;
		}

		// get extension infos
		QVulkanInfoVector<QVulkanExtension> vkexts = vk->supportedExtensions();
		for(const auto& vkext : vkexts)
		{
			std::cout << "Vk extension: " << vkext.name.toStdString()
				<< ", version " << vkext.version << "." << std::endl;
		}

		// create main and vk window
		auto vkwnd = new VkWnd(vk);
		auto wnd = std::make_unique<Wnd>(vkwnd);
		wnd->resize(800, 600);
		wnd->show();
		// ------------------------------------------------------------------------


		// run application
		return app->exec();
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return -1;
	}
}
// ----------------------------------------------------------------------------
