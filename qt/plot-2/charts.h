/**
 * plot test
 * @author Tobias Weber
 * @date 30-Mar-2018
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __QTTST_H__
#define __QTTST_H__

#include <QDialog>
#include <QChartView>


class MyChartView : public QtCharts::QChartView
{ Q_OBJECT
public:
	MyChartView(QWidget *pParent);
	virtual ~MyChartView() {}

protected:
	virtual void mouseMoveEvent(QMouseEvent*) override;

signals:
	void mouseMoveEventSignal(QMouseEvent*);
};


class PltDlg : public QDialog
{
public:
	using QDialog::QDialog;
	PltDlg(QWidget* pParent = nullptr);
	~PltDlg() = default;

private:
	QtCharts::QChart *m_pChart = nullptr;
	MyChartView *m_pChartView = nullptr;

	void plotMouseMove(QMouseEvent*);
};

#endif
