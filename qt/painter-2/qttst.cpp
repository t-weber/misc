/**
 * painter test
 * @author Tobias Weber
 * @date May-2020
 * @license: see 'LICENSE.GPL' file
 */
#include "qttst.h"

#include <QApplication>
#include <QGridLayout>
#include <QPainter>

#include <locale>
#include <iostream>


Widget::Widget(QWidget *pParent)
	: m_up{0}, m_down{0}, m_left{0}, m_right{0},
		m_pos{0., 0.}, m_dir{0., 0.},
		m_angle{0.}, m_fov(M_PI/2.)
{
	{
		t_poly poly;
		poly.outer().push_back(t_vertex{-0.25, -0.25});
		poly.outer().push_back(t_vertex{0.25, -0.25});
		poly.outer().push_back(t_vertex{0.25, 0.25});
		poly.outer().push_back(t_vertex{-0.25, 0.25});
		m_geo.emplace_back(std::move(poly));
	}

	{
		t_poly poly;
		poly.outer().push_back(t_vertex{-0.02, -0.02});
		poly.outer().push_back(t_vertex{0.02, -0.02});
		poly.outer().push_back(t_vertex{0.02, 0.02});
		poly.outer().push_back(t_vertex{-0.02, 0.02});
		m_geo.emplace_back(std::move(poly));
	}

	setFocusPolicy(Qt::StrongFocus);	// to receive keyboard events
	setMouseTracking(true);

	connect(&m_timer, &QTimer::timeout, this, static_cast<void (Widget::*)()>(&Widget::tick));
	m_timer.start(std::chrono::milliseconds(1000 / 60));
}


Widget::~Widget()
{
	m_timer.stop();
}


void Widget::resizeEvent(QResizeEvent *pEvt)
{
	m_screenDims[0] = pEvt->size().width();
	m_screenDims[1] = pEvt->size().height();

	std::cerr << std::dec << __func__ << ": w = " << m_screenDims[0]
		<< ", h = " << m_screenDims[1] << std::endl;
}


void Widget::mouseMoveEvent(QMouseEvent *pEvt)
{
	m_posMouse = pEvt->localPos();
	//std::cout << "pos = (" << m_posMouse.x() << " " << m_posMouse.y() << ")" << std::endl;
}


void Widget::keyPressEvent(QKeyEvent* pEvt)
{
	switch(pEvt->key())
	{
		case Qt::Key_Up: m_up = 1; break;
		case Qt::Key_Down: m_down = 1; break;
		case Qt::Key_Left: m_left = 1; break;
		case Qt::Key_Right: m_right = 1; break;
	}
}


void Widget::keyReleaseEvent(QKeyEvent* pEvt)
{
	switch(pEvt->key())
	{
		case Qt::Key_Up: m_up = 0; break;
		case Qt::Key_Down: m_down = 0; break;
		case Qt::Key_Left: m_left = 0; break;
		case Qt::Key_Right: m_right = 0; break;
	}
}



void Widget::paintEvent(QPaintEvent *pEvt)
{
	QPainter painter(this);


	t_real column_w = 1. / t_real(m_casted.size());

	for(std::size_t idx=0; idx<m_casted.size(); ++idx)
	{
		t_real x = t_real(idx)/t_real(m_casted.size()) - 0.5;
		t_real h = m_casted[idx];

		QVector2D topL{x, h*0.5f};
		QVector2D bottomR{x+column_w, -h*0.5f};

		QPointF ptTopL = ToScreenCoords(topL);
		QPointF ptBottomR = ToScreenCoords(bottomR);
		painter.drawRect(QRectF{ptTopL, ptBottomR});
	}


	// draw geometry
	for(const t_poly& poly : m_geo)
	{
		for(std::size_t i=0; i<poly.outer().size(); ++i)
		{
			std::size_t j = (i+1<poly.outer().size() ? i+1 : 0);
			const t_vertex& vert1 = poly.outer()[i];
			const t_vertex& vert2 = poly.outer()[j];

			painter.drawLine(ToSidescreenCoords(QVector2D(vert1.get<0>(), vert1.get<1>())),
				ToSidescreenCoords(QVector2D(vert2.get<0>(), vert2.get<1>())));
		}
	}


	// position and direction of camera
	painter.drawEllipse(ToSidescreenCoords(m_pos), 2.5, 2.5);
	//painter.drawLine(ToSidescreenCoords(m_pos), ToSidescreenCoords(m_pos + m_dir*0.05));

	// fov
	painter.drawLine(ToSidescreenCoords(m_pos), ToSidescreenCoords(m_pos + m_fovlines[0]*0.05f));
	painter.drawLine(ToSidescreenCoords(m_pos), ToSidescreenCoords(m_pos + m_fovlines[1]*0.05f));
}


void Widget::tick()
{
	tick(std::chrono::milliseconds(1000 / 60));
}


void Widget::tick(const std::chrono::milliseconds& ms)
{
	if(m_right)
		m_angle -= 3e-3*t_real(ms.count());
	if(m_left)
		m_angle += 3e-3*t_real(ms.count());

	// update position and fov angle
	m_dir = QVector2D{std::cos(m_angle), std::sin(m_angle)};
	m_fovlines[0] = QVector2D{std::cos(m_angle-m_fov*0.5f), std::sin(m_angle-m_fov*0.5f)};
	m_fovlines[1] = QVector2D{std::cos(m_angle+m_fov*0.5f), std::sin(m_angle+m_fov*0.5f)};

	if(m_up)
		m_pos += m_dir * 2e-4*t_real(ms.count());
	if(m_down)
		m_pos -= m_dir * 2e-4*t_real(ms.count());


	// fov ray intersections
	t_real NUM_ANGLES = 100.f;
	m_casted.resize(std::size_t(NUM_ANGLES));
	std::size_t idx = 0;

	for(t_real angle=m_angle+m_fov*0.5; angle>=m_angle-m_fov*0.5; angle-=m_fov/NUM_ANGLES)
	{
		t_lines fovline;
		fovline.push_back(t_vertex{m_pos[0], m_pos[1]});
		fovline.push_back(t_vertex{m_pos[0] + std::cos(angle), m_pos[1] + std::sin(angle)});

		// intersect with all geometry objects and find closest
		t_real min = std::numeric_limits<t_real>::max();
		for(const t_poly& poly : m_geo)
		{
			std::vector<t_vertex> vecPts;
			geo::intersection(poly, fovline, vecPts);
			for(const t_vertex& vert : vecPts)
			{
				QVector2D dist{vert.get<0>() - m_pos[0], vert.get<1>()-m_pos[1]};
				t_real distToCam = dist.length();
				min = std::min(min, distToCam);
			}
		}

		double scale = 0.1;
		if(idx >= m_casted.size())
			break;
		m_casted[idx] = scale/(min);
		++idx;

		//std::cout << m_casted[idx] << " ";
	}
	//std::cout << std::endl;

	update();
}


QPointF Widget::ToScreenCoords(const QVector2D& vec)
{
	return QPointF(
		(vec[0]+0.5)*m_screenDims[0],
		m_screenDims[1] - (vec[1]+0.5)*m_screenDims[1]);
}


QPointF Widget::ToSidescreenCoords(const QVector2D& vec)
{
	t_real scale = 0.3;
	return QPointF(
		m_screenDims[0] - (vec[0]+0.5)*m_screenDims[0]*0.3,
		(vec[1]+0.5)*m_screenDims[1]*0.3);
}

// ----------------------------------------------------------------------------




// ----------------------------------------------------------------------------
TstDlg::TstDlg(QWidget* pParent) : QDialog{pParent},
	m_pWidget{new Widget(this)}
{
	auto pGrid = new QGridLayout(this);
	pGrid->setSpacing(2);
	pGrid->setContentsMargins(4,4,4,4);
	pGrid->addWidget(m_pWidget.get(), 0,0,1,1);
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
	dlg->resize(800, 800);
	dlg->show();

	return app->exec();
}
// ----------------------------------------------------------------------------
