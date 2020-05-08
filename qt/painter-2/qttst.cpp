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
	t_poly poly;
	poly.outer().push_back(t_vertex{-0.25, -0.25});
	poly.outer().push_back(t_vertex{0.25, -0.25});
	poly.outer().push_back(t_vertex{0.25, 0.25});
	poly.outer().push_back(t_vertex{-0.25, 0.25});
	m_geo.emplace_back(std::move(poly));

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

	for(const t_poly& poly : m_geo)
	{
		for(std::size_t i=0; i<poly.outer().size(); ++i)
		{
			std::size_t j = (i+1<poly.outer().size() ? i+1 : 0);
			const t_vertex& vert1 = poly.outer()[i];
			const t_vertex& vert2 = poly.outer()[j];

			painter.drawLine(ToScreenCoords(QVector2D(vert1.get<0>(), vert1.get<1>())),
				ToScreenCoords(QVector2D(vert2.get<0>(), vert2.get<1>())));
		}
	}


	// position and direction of camera
	painter.drawEllipse(ToScreenCoords(m_pos), 5., 5.);
	//painter.drawLine(ToScreenCoords(m_pos), ToScreenCoords(m_pos + m_dir*0.05));

	// fov
	painter.drawLine(ToScreenCoords(m_pos), ToScreenCoords(m_pos + m_fovlines[0]*0.05f));
	painter.drawLine(ToScreenCoords(m_pos), ToScreenCoords(m_pos + m_fovlines[1]*0.05f));
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

	m_dir = QVector2D{std::cos(m_angle), std::sin(m_angle)};
	m_fovlines[0] = QVector2D{std::cos(m_angle-m_fov*0.5f), std::sin(m_angle-m_fov*0.5f)};
	m_fovlines[1] = QVector2D{std::cos(m_angle+m_fov*0.5f), std::sin(m_angle+m_fov*0.5f)};

	if(m_up)
		m_pos += m_dir * 2e-4*t_real(ms.count());
	if(m_down)
		m_pos -= m_dir * 2e-4*t_real(ms.count());


	// intersection
	/*t_lines fovline;
	fovline.push_back(t_vertex{m_pos[0], m_pos[1]});
	fovline.push_back(t_vertex{m_pos[0] + m_fovlines[0][0], m_pos[1] + m_fovlines[0][1]});
	std::vector<t_vertex> vecPts;
	geo::intersection(m_geo[0], fovline, vecPts);
	for(const t_vertex& vert : vecPts)
		std::cout << vert.get<0>() << " " << vert.get<1>() << std::endl;*/

	update();
}


QPointF Widget::ToScreenCoords(const QVector2D& vec)
{
	return QPointF(
		(vec[0]+0.5)*m_screenDims[0],
		m_screenDims[1] - (vec[1]+0.5)*m_screenDims[1]);
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
