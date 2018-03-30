/**
 * plot test
 * @author Tobias Weber
 * @date 30-Mar-2018
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __QTTST_H__
#define __QTTST_H__

#include <QDialog>
#include "qcustomplot/qcustomplot.h"


class PltDlg : public QDialog
{
public:
	using QDialog::QDialog;
	PltDlg(QWidget* pParent = nullptr);
	~PltDlg() = default;

private:
	void plotMouseMove(QMouseEvent*);

private:
	QCustomPlot *m_pPlot = nullptr;
};

#endif
