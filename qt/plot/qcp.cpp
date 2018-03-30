/**
 * plot test
 * @author Tobias Weber
 * @date 30-Mar-2018
 * @license: see 'LICENSE.GPL' file
 */

#include "qcp.h"
#include <QApplication>
#include <QGridLayout>

#include <locale>
#include <memory>
#include <iostream>


// ----------------------------------------------------------------------------

// internal data type of plotter is double
using t_real = double;


template<class t_cont = std::initializer_list<t_real>>
QVector<t_real> to_qvec(const t_cont& vec)
{
	QVector<t_real> qvec;

	for(t_real d : vec)
		qvec << d;

	return qvec;
}

// ----------------------------------------------------------------------------


PltDlg::PltDlg(QWidget* pParent) : QDialog{pParent}
{
	this->resize(800, 600);

	auto pLayout = new QGridLayout(this);
	m_pPlot = new QCustomPlot(this);
	pLayout->addWidget(m_pPlot, 0, 0, 1, 1);


	auto x = to_qvec({1, 2, 3, 4});
	auto y = to_qvec({10, 15, 32, 38});
	auto yerr = to_qvec({0.4, 0.7, 1.2, 1.6});

	auto [xmin, xmax] = std::minmax_element(x.begin(), x.end());
	auto [ymin, ymax] = std::minmax_element(y.begin(), y.end());

	auto pGraph = m_pPlot->addGraph();
	pGraph->setLineStyle(QCPGraph::lsNone);
	pGraph->setScatterStyle(QCPScatterStyle::ssCircle);
	pGraph->setData(x, y);

	auto pErr = new QCPErrorBars(m_pPlot->xAxis, m_pPlot->yAxis);
	pErr->setData(yerr);
	pErr->setDataPlottable(pGraph);
	
	m_pPlot->xAxis->setRange(*xmin-*xmax/10., *xmax+*xmax/10.);
	m_pPlot->yAxis->setRange(*ymin-*ymax/10., *ymax+*ymax/10.);

	m_pPlot->setSelectionRectMode(QCP::srmZoom);
	m_pPlot->setInteraction(QCP::Interaction(int(QCP::iRangeZoom) | int(QCP::iRangeDrag)));
	m_pPlot->replot();


	connect(m_pPlot, &QCustomPlot::mouseMove, this, &PltDlg::plotMouseMove);
}


void PltDlg::plotMouseMove(QMouseEvent *pEvt)
{
	if(!m_pPlot) return;

	t_real x = m_pPlot->xAxis->pixelToCoord(pEvt->x());
	t_real y = m_pPlot->yAxis->pixelToCoord(pEvt->y());

	QString strCoord = (std::to_string(x) + ", " + std::to_string(y)).c_str();
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
