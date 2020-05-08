/**
 * qt non-gl painter example
 * @author Tobias Weber
 * @date Nov-2017
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __QTTST_H__
#define __QTTST_H__


#include <QDialog>
#include <QTimer>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>

#include <QVector2D>

#include <memory>
#include <chrono>
#include <vector>

#include <boost/geometry.hpp>
namespace geo = boost::geometry;


using t_real = float;
using t_vertex = geo::model::point<t_real, 2, geo::cs::cartesian>;
using t_poly = geo::model::polygon<t_vertex, true, false>;
using t_lines = geo::model::linestring<t_vertex>;


class Widget : public QWidget
{
public:
	using QWidget::QWidget;

	Widget(QWidget *pParent);
	virtual ~Widget();

	QPointF ToScreenCoords(const QVector2D& vec);

protected:
	virtual void resizeEvent(QResizeEvent *pEvt) override;
	virtual void paintEvent(QPaintEvent *pEvt) override;
	virtual void mouseMoveEvent(QMouseEvent *pEvt) override;
	virtual void keyPressEvent(QKeyEvent* pEvt) override;
	virtual void keyReleaseEvent(QKeyEvent* pEvt) override;

	void tick(const std::chrono::milliseconds& ms);

private:
	t_real m_screenDims[2] = { -1, -1 };
	QTimer m_timer;
	QPointF m_posMouse;

	std::vector<t_poly> m_geo;

	bool m_up, m_down, m_left, m_right;

	// camera position and angle
	QVector2D m_pos, m_dir;
	t_real m_angle, m_fov;
	QVector2D m_fovlines[2];

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
