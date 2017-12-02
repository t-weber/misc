/**
 * qt non-gl painter example
 * @author Tobias Weber
 * @date Nov-2017
 */

#ifndef __QTTST_H__
#define __QTTST_H__

// ----------------------------------------------------------------------------

#include <QDialog>
#include <QTimer>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>

#include <QMatrix4x4>
#include <QVector4D>
#include <QVector3D>

#include <memory>
#include <chrono>


class Widget : public QWidget
{
public:
	using QWidget::QWidget;

	Widget(QWidget *pParent);
	virtual ~Widget();

	QPointF ToScreenCoords(const QVector3D& vec, bool *pVisible=nullptr);

protected:
	virtual void resizeEvent(QResizeEvent *pEvt) override;
	virtual void paintEvent(QPaintEvent *pEvt) override;
	virtual void mouseMoveEvent(QMouseEvent *pEvt) override;

	void tick(const std::chrono::milliseconds& ms);

private:
	QMatrix4x4 m_matPerspective, m_matViewport, m_matCam;

	int m_iScreenDims[2] = { -1, -1 };
	QTimer m_timer;
	QPointF m_posMouse;

protected slots:
	void tick();
};


class TstDlg : public QDialog
{
public:
	using QDialog::QDialog;
	TstDlg(QWidget* pParent);
	~TstDlg() = default;

private:
	std::shared_ptr<Widget> m_pWidget;
};


#endif
