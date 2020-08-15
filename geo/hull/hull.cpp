/**
 * convex hull test program
 * @author Tobias Weber
 * @date 15-Aug-2020
 * @license: see 'LICENSE.GPL' file
 */

#include "hull.h"

#include <QApplication>
#include <QMouseEvent>

#include <locale>
#include <memory>
#include <iostream>

#include <libqhullcpp/Qhull.h>
#include <libqhullcpp/QhullFacet.h>
#include <libqhullcpp/QhullFacetList.h>
#include <libqhullcpp/QhullVertexSet.h>

namespace qh = orgQhull;
using t_real = coordT;


// ----------------------------------------------------------------------------

Vertex::Vertex()
{
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


Vertex::~Vertex()
{
}


QRectF Vertex::boundingRect() const
{
	return QRectF{-10, -10, 10, 10};
}


void Vertex::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	painter->drawEllipse(-10, -10, 10, 10);
}

// ----------------------------------------------------------------------------




// ----------------------------------------------------------------------------

HullView::HullView(QGraphicsScene *scene, QWidget *parent) : QGraphicsView(scene, parent),
	m_scene{scene}
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	setMouseTracking(true);
}


HullView::~HullView()
{
}


void HullView::mousePressEvent(QMouseEvent *evt)
{
	QPoint posVP = evt->pos();
	QPointF posScene = mapToScene(posVP);

	QGraphicsItem* item = itemAt(posVP);
	bool item_is_vertex = m_vertices.find(static_cast<const Vertex*>(item)) != m_vertices.end();

	if(evt->button() == Qt::LeftButton)
	{
		// if no vertex is at this position, create a new one
		if(!item)
		{
			Vertex *vertex = new Vertex{};
			m_vertices.insert(vertex);
			vertex->setPos(posScene);
			m_scene->addItem(vertex);

			m_dragging = true;
		}

		else
		{
			// vertex is being dragged
			if(item_is_vertex)
				m_dragging = true;
		}
	}
	else if(evt->button() == Qt::RightButton)
	{
		// if a vertex is at this position, remove it
		if(item && item_is_vertex)
		{
			m_scene->removeItem(item);
			m_vertices.erase(static_cast<const Vertex*>(item));
			delete item;
		}
	}

	UpdateHull();
	QGraphicsView::mousePressEvent(evt);
}


void HullView::mouseReleaseEvent(QMouseEvent *evt)
{
	if(evt->button() == Qt::LeftButton)
		m_dragging = false;

	QGraphicsView::mouseReleaseEvent(evt);
}


void HullView::mouseMoveEvent(QMouseEvent *evt)
{
	QGraphicsView::mouseMoveEvent(evt);

	if(m_dragging)
		UpdateHull();
}


static double side_of_line(const QLineF& line, const QPointF& pt)
{
	QPointF dir1 = line.p2() - line.p1();
	QPointF dir2 = pt - line.p1();

	return dir1.x()*dir2.y() - dir1.y()*dir2.x();
}


static bool all_points_on_same_side(const QLineF& line, const std::vector<QPointF>& hullvertices)
{
	double eps = 1e-5;

	// find a reference vertex which is sufficiently far from the line
	std::optional<double> side;
	for(const QPointF& vert : hullvertices)
	{
		if(!side)
		{
			double curside = side_of_line(line, vert);
			if(std::abs(curside) > eps)
				side = curside;
		}

		if(side)
			break;
	}

	if(!side)
		return true;


	// are all other vertices on the same sine as the reference vertex (or on the line)?
	for(const QPointF& vert : hullvertices)
	{
		double curside = side_of_line(line, vert);
		if(std::signbit(*side) != std::signbit(curside) && std::abs(curside) > eps)
			return false;
}

	return true;
}


void HullView::UpdateHull()
{
	// remove previous hull
	for(QGraphicsItem* hullItem : m_hull)
	{
		m_scene->removeItem(hullItem);
		delete hullItem;
	}
	m_hull.clear();


	if(m_vertices.size() < 3)
		return;

	std::vector<t_real> vertices;
	vertices.reserve(m_vertices.size()*2);
	for(const Vertex* vertex : m_vertices)
	{
		vertices.push_back(vertex->x());
		vertices.push_back(vertex->y());
	}


	std::vector<t_real> hull = CalcHull<t_real>(2, vertices);

	std::vector<QPointF> hullvertices;
	for(std::size_t i=0; i<hull.size(); i+=2)
		hullvertices.emplace_back(QPointF{hull[i], hull[i+1]});


	for(std::size_t idx1=0; idx1<hullvertices.size(); ++idx1)
	{
		std::size_t idx2 = idx1+1;
		if(idx2 >= hullvertices.size())
			idx2 = 0;

		QLineF line{hullvertices[idx1], hullvertices[idx2]};
		if(!all_points_on_same_side(line, hullvertices))
			continue;

		QGraphicsItem *hullItem = m_scene->addLine(line);
		m_hull.insert(hullItem);
	}
}


template<class t_real=double>
std::vector<t_real> HullView::CalcHull(int dim, const std::vector<t_real>& vec)
{
	std::vector<t_real> hull;

	qh::Qhull qh{"hull", dim, int(vec.size()/dim), vec.data(), "QJ" };
	qh::QhullVertexList vertices{qh.vertexList()};

	qh::QhullFacetList facets{qh.facetList()};
	for(auto iterFacet=facets.begin(); iterFacet!=facets.end(); ++iterFacet)
	{
		qh::QhullVertexSet vertices = iterFacet->vertices();

		for(auto iterVertex=vertices.begin(); iterVertex!=vertices.end(); ++iterVertex)
		{
			qh::QhullPoint pt = (*iterVertex).point();

			for(int i=0; i<dim; ++i)
				hull.push_back(pt[i]);
		}
	}

	return hull;
}


// ----------------------------------------------------------------------------




// ----------------------------------------------------------------------------

HullWnd::HullWnd(QWidget* pParent) : QMainWindow{pParent},
	m_scene{new QGraphicsScene{this}},
	m_view{new HullView{m_scene.get(), this}}
{
	setWindowTitle("Hull");
	setCentralWidget(m_view.get());
}


HullWnd::~HullWnd()
{
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

	auto hullwnd = std::make_unique<HullWnd>();
	hullwnd->resize(800, 600);
	hullwnd->show();

	return app->exec();
}
// ----------------------------------------------------------------------------
