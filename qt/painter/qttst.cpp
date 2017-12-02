/**
 * qt non-gl painter example
 * @author Tobias Weber
 * @date Nov-2017
 */
#include "qttst.h"

#include <QApplication>
#include <QGridLayout>
#include <QPainter>

#include <locale>
#include <iostream>


Widget::Widget(QWidget *pParent)
{
	setMouseTracking(1);

	connect(&m_timer, &QTimer::timeout, this, static_cast<void (Widget::*)()>(&Widget::tick));
	m_timer.start(std::chrono::milliseconds(1000 / 60));
}


Widget::~Widget()
{
	m_timer.stop();
}


void Widget::resizeEvent(QResizeEvent *pEvt)
{
	m_iScreenDims[0] = pEvt->size().width();
	m_iScreenDims[1] = pEvt->size().height();

	std::cerr << std::dec << __func__ << ": w = " << m_iScreenDims[0]
	<< ", h = " << m_iScreenDims[1] << std::endl;

	m_matViewport.setToIdentity();
	m_matViewport.viewport(0, 0, m_iScreenDims[0], m_iScreenDims[1], 0., 1.);

	m_matPerspective.setToIdentity();
	m_matPerspective.perspective(90., double(m_iScreenDims[0])/double(m_iScreenDims[1]), 0.01, 100.);
}


void Widget::mouseMoveEvent(QMouseEvent *pEvt)
{
	m_posMouse = pEvt->localPos();
}


void Widget::paintEvent(QPaintEvent *pEvt)
{
	QPainter painter(this);

	painter.drawLine(ToScreenCoords(QVector3D(-0.5, 0., -1.)), ToScreenCoords(QVector3D(0.5, 0., -2.)));
	painter.drawLine(ToScreenCoords(QVector3D(0.5, 0., -2.)), ToScreenCoords(QVector3D(0.5, 0.5, -1.)));
	painter.drawLine(ToScreenCoords(QVector3D(0.5, 0.5, -1.)), ToScreenCoords(QVector3D(-0.5, 0., -1.)));

	painter.drawEllipse(m_posMouse, 5., 5.);

	painter.drawText(ToScreenCoords(QVector3D(-0.5,0.,-1.)), "* Vertex 1");
	painter.drawText(ToScreenCoords(QVector3D(0.5, 0., -2.)), "* Vertex 2");
	painter.drawText(ToScreenCoords(QVector3D(0.5, 0.5, -1.)), "* Vertex 3");
}


void Widget::tick()
{
	tick(std::chrono::milliseconds(1000 / 60));
}

void Widget::tick(const std::chrono::milliseconds& ms)
{
	m_matCam.rotate(1.5, 0.,0.,1.);
	update();
}


QPointF Widget::ToScreenCoords(const QVector3D& vec3, bool *pVisible)
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
	dlg->resize(800, 600);
	dlg->show();

	return app->exec();
}
// ----------------------------------------------------------------------------
