/**
 * plot test
 * @author Tobias Weber
 * @date 30-Mar-2018
 * @license: see 'LICENSE.GPL' file
 *
 * Reference: https://github.com/qt/qtcharts
 */

#include "charts.h"
#include <QApplication>
#include <QGridLayout>

#include <locale>
#include <memory>
#include <iostream>

#include <QLineSeries>
#include <QScatterSeries>
//#include <QBoxPlotSeries>


// ----------------------------------------------------------------------------

// internal data type of plotter is double
using t_real = double;


// ----------------------------------------------------------------------------


MyChartView::MyChartView(QWidget *pParent) : QtCharts::QChartView(pParent)
{}


void MyChartView::mouseMoveEvent(QMouseEvent* pEvt)
{
	emit mouseMoveEventSignal(pEvt);
	QtCharts::QChartView::mouseMoveEvent(pEvt);
}


// ----------------------------------------------------------------------------


PltDlg::PltDlg(QWidget* pParent) : QDialog{pParent}
{
	this->resize(800, 600);

	m_pChartView = new MyChartView(this);
	m_pChartView->setRenderHint(QPainter::Antialiasing);
	m_pChartView->setRubberBand(QtCharts::QChartView::RectangleRubberBand);

	auto pLayout = new QGridLayout(this);
	pLayout->addWidget(m_pChartView, 0, 0, 1, 1);

	m_pChart = new QtCharts::QChart();
	m_pChartView->setChart(m_pChart);

	auto x =  std::vector<t_real>({1, 2, 3, 4});
	auto y = std::vector<t_real>({12, 9, 28, 42});
	auto yerr = std::vector<t_real>({0.5, 0.4, 5., 0.8});

	auto pLine = new QtCharts::QLineSeries();
	pLine->setUseOpenGL(1);
	pLine->setColor(QColor(0xff, 0x00, 0x00, 0xff));
	QPen penLine = pLine->pen();
	penLine.setWidth(2.);
	pLine->setPen(penLine);

	auto pPoints = new QtCharts::QScatterSeries();
	pPoints->setUseOpenGL(1);
	pPoints->setMarkerShape(QtCharts::QScatterSeries::MarkerShapeCircle);
	pPoints->setMarkerSize(16.);
	pPoints->setColor(QColor(0x00, 0x00, 0x00, 0xff));
	pPoints->setBorderColor(QColor(0xff, 0xff, 0xff, 0xff));
	pPoints->setName("data");

	//auto pErrBars = new QtCharts::QBoxPlotSeries();
	//pErrBars->setUseOpenGL(1);


	for(std::size_t i=0; i<x.size(); ++i)
	{
		pLine->append(x[i], y[i]);
		pPoints->append(x[i], y[i]);
		//pErrBars->append(new QtCharts::QBoxSet(y[i]-0.2, y[i]-0.1, y[i], y[i]+0.1, y[i]+0.2, "123"));
	}

	m_pChart->addSeries(pLine);
	m_pChart->addSeries(pPoints);
	//m_pChart->addSeries(pErrBars);


	m_pChart->createDefaultAxes();
	m_pChart->legend()->setVisible(0);

	connect(m_pChartView, &MyChartView::mouseMoveEventSignal, this, &PltDlg::plotMouseMove);
}


void PltDlg::plotMouseMove(QMouseEvent* pEvt)
{
	if(!m_pChart) return;

	QPointF pos = m_pChart->mapToValue(pEvt->pos());

    QString strCoord = (std::to_string(pos.x()) + ", " + std::to_string(pos.y())).c_str();
    setWindowTitle(strCoord);
}

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

	auto dlg = std::make_unique<PltDlg>();
	dlg->show();

	return app->exec();
}

// ----------------------------------------------------------------------------
