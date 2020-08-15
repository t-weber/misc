/**
 * convex hull test program
 * @author Tobias Weber
 * @date 15-Aug-2020
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __HULLTST_H__
#define __HULLTST_H__


#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>

#include <memory>
#include <unordered_set>
#include <vector>



class Vertex : public QGraphicsItem
{
public:
	Vertex();
	virtual ~Vertex();

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

};



class HullView : public QGraphicsView
{
public:
	HullView(QGraphicsScene *scene=nullptr, QWidget *parent=nullptr);
	virtual ~HullView();

	HullView(HullView&) = delete;
	const HullView& operator=(const HullView&) const = delete;

	void SetCalculateHull(bool b);
	void ClearVertices();

protected:
	template<class t_real=double>
	static std::vector<t_real> CalcHull(int dim, const std::vector<t_real>& vec);

protected:
	virtual void mousePressEvent(QMouseEvent *evt) override;
	virtual void mouseReleaseEvent(QMouseEvent *evt) override;
	virtual void mouseMoveEvent(QMouseEvent *evt) override;

	virtual void resizeEvent(QResizeEvent *evt) override;
	void UpdateHull();

private:
	QGraphicsScene *m_scene = nullptr;

	std::unordered_set<Vertex*> m_vertices{};
	std::unordered_set<QGraphicsItem*> m_hull{};

	bool m_dragging = false;
	bool m_calchull = true;
};



class HullWnd : public QMainWindow
{
public:
	using QMainWindow::QMainWindow;

	HullWnd(QWidget* pParent = nullptr);
	~HullWnd();

private:
	std::shared_ptr<QGraphicsScene> m_scene;
	std::shared_ptr<HullView> m_view;
};


#endif
