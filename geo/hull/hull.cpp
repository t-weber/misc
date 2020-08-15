/**
 * convex hull test program
 * @author Tobias Weber
 * @date 15-Aug-2020
 * @license: see 'LICENSE.GPL' file
 */

#include "hull.h"

#include <QApplication>
#include <QMenuBar>
#include <QMouseEvent>

#include <locale>
#include <memory>
#include <array>
#include <iostream>

#include <libqhullcpp/Qhull.h>
#include <libqhullcpp/QhullFacet.h>
#include <libqhullcpp/QhullFacetList.h>
#include <libqhullcpp/QhullVertexSet.h>

namespace qh = orgQhull;
using t_real = coordT;


// ----------------------------------------------------------------------------

Vertex::Vertex(const QPointF& pos, double rad) : m_rad{rad}
{
	setPos(pos);
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


Vertex::~Vertex()
{
}


QRectF Vertex::boundingRect() const
{
	return QRectF{-m_rad/2., -m_rad/2., m_rad, m_rad};
}


void Vertex::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	std::array<QColor, 2> colours =
	{
		QColor::fromRgbF(0.,0.,1.),
		QColor::fromRgbF(0.,0.,0.),
	};

	QRadialGradient grad{};
	grad.setCenter(0., 0.);
	grad.setRadius(m_rad);

	for(std::size_t col=0; col<colours.size(); ++col)
		grad.setColorAt(col/double(colours.size()-1), colours[col]);

	painter->setBrush(grad);
	painter->setPen(*colours.rbegin());

	painter->drawEllipse(-m_rad/2., -m_rad/2., m_rad, m_rad);
}

// ----------------------------------------------------------------------------




// ----------------------------------------------------------------------------

HullView::HullView(QGraphicsScene *scene, QWidget *parent) : QGraphicsView(scene, parent),
	m_scene{scene}
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	setInteractive(true);
	setMouseTracking(true);
}


HullView::~HullView()
{
}


void HullView::resizeEvent(QResizeEvent *evt)
{
	QPointF pt1{mapToScene(QPoint{0,0})};
	QPointF pt2{mapToScene(QPoint{evt->size().width(), evt->size().height()})};

	const double padding = 16;

	// include bounds given by vertices
	for(const Vertex* vertex : m_vertices)
	{
		QPointF vertexpos = vertex->scenePos();

		if(vertexpos.x() < pt1.x())
			pt1.setX(vertexpos.x() -  padding);
		if(vertexpos.x() > pt2.x())
			pt2.setX(vertexpos.x() +  padding);
		if(vertexpos.y() < pt1.y())
			pt1.setY(vertexpos.y() -  padding);
		if(vertexpos.y() > pt2.y())
			pt2.setY(vertexpos.y() +  padding);
	}

	setSceneRect(QRectF{pt1, pt2});
}



void HullView::mousePressEvent(QMouseEvent *evt)
{
	QPoint posVP = evt->pos();
	QPointF posScene = mapToScene(posVP);

	QGraphicsItem* item = itemAt(posVP);
	bool item_is_vertex = m_vertices.find(static_cast<Vertex*>(item)) != m_vertices.end();

	if(evt->button() == Qt::LeftButton)
	{
		// if no vertex is at this position, create a new one
		if(!item)
		{
			Vertex *vertex = new Vertex{posScene};
			m_vertices.insert(vertex);
			m_scene->addItem(vertex);

			m_dragging = true;
			UpdateAll();
		}

		else
		{
			// vertex is being dragged
			//if(item_is_vertex)
			{
				m_dragging = true;
			}
		}
	}
	else if(evt->button() == Qt::RightButton)
	{
		// if a vertex is at this position, remove it
		if(item && item_is_vertex)
		{
			m_scene->removeItem(item);
			m_vertices.erase(static_cast<Vertex*>(item));
			delete item;
		}
	}

	UpdateAll();
	QGraphicsView::mousePressEvent(evt);
}


void HullView::mouseReleaseEvent(QMouseEvent *evt)
{
	if(evt->button() == Qt::LeftButton)
		m_dragging = false;

	UpdateAll();
	QGraphicsView::mouseReleaseEvent(evt);
}


void HullView::mouseMoveEvent(QMouseEvent *evt)
{
	QGraphicsView::mouseMoveEvent(evt);

	if(m_dragging)
	{
		QResizeEvent evt{size(), size()};
		resizeEvent(&evt);
		UpdateAll();
	}
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


void HullView::SetCalculateHull(bool b)
{
	m_calchull = b;
	UpdateHull();
}


void HullView::SetCalculateVoronoi(bool b)
{
	m_calcvoronoi = b;
	UpdateVoronoi();
}


void HullView::ClearVertices()
{
	for(Vertex* vertex : m_vertices)
	{
		m_scene->removeItem(vertex);
		delete vertex;
	}
	m_vertices.clear();

	UpdateAll();
}


void HullView::UpdateAll()
{
	UpdateHull();
	UpdateVoronoi();
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

	if(!m_calchull || m_vertices.size() < 3)
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


void HullView::UpdateVoronoi()
{
	// remove previous voronoi vertices
	for(QGraphicsItem* voronoiItem : m_voronoi)
	{
		m_scene->removeItem(voronoiItem);
		delete voronoiItem;
	}
	m_voronoi.clear();

	if(!m_calcvoronoi || m_vertices.size() < 4)
		return;

	std::vector<t_real> vertices;
	vertices.reserve(m_vertices.size()*2);
	for(const Vertex* vertex : m_vertices)
	{
		vertices.push_back(vertex->x());
		vertices.push_back(vertex->y());
	}


	std::vector<t_real> voronoi = CalcVoronoi<t_real>(2, vertices);

	for(std::size_t i=0; i<voronoi.size(); i+=2)
	{
		QGraphicsItem *voronoiItem = m_scene->addEllipse(voronoi[i], voronoi[i+1], 7, 7);
		m_voronoi.insert(voronoiItem);
	}
}


template<class t_real>
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


template<class t_real>
std::vector<t_real> HullView::CalcVoronoi(int dim, const std::vector<t_real>& vec)
{
	std::vector<t_real> voronoi;

	qh::Qhull qh{"voronoi", dim, int(vec.size()/dim), vec.data(), "v" };
	qh::QhullVertexList vertices{qh.vertexList()};

	qh::QhullFacetList facets{qh.facetList()};
	for(auto iterFacet=facets.begin(); iterFacet!=facets.end(); ++iterFacet)
	{
		qh::QhullPoint pt = iterFacet->voronoiVertex();

		for(int i=0; i<dim; ++i)
			voronoi.push_back(pt[i]);
	}

	return voronoi;
}


// ----------------------------------------------------------------------------




// ----------------------------------------------------------------------------

HullWnd::HullWnd(QWidget* pParent) : QMainWindow{pParent},
	m_scene{new QGraphicsScene{this}},
	m_view{new HullView{m_scene.get(), this}}
{
	setWindowTitle("Hull");
	setCentralWidget(m_view.get());


	// menu
	QMenu *menuFile = new QMenu{"File", this};
	QMenu *menuCalc = new QMenu{"Calculate", this};

	QAction *actionNew = new QAction{"New", this};
	connect(actionNew, &QAction::triggered, [this](){ m_view->ClearVertices(); });
	menuFile->addAction(actionNew);
	menuFile->addSeparator();

	QAction *actionQuit = new QAction{"Exit", this};
	connect(actionQuit, &QAction::triggered, [this](){ this->close(); });
	menuFile->addAction(actionQuit);

	QAction *actionHull = new QAction{"Convex Hull", this};
	actionHull->setCheckable(true);
	actionHull->setChecked(true);
	connect(actionHull, &QAction::toggled, [this](bool b) { m_view->SetCalculateHull(b); });
	menuCalc->addAction(actionHull);

	QAction *actionVoronoi = new QAction{"Voronoi Vertices", this};
	actionVoronoi->setCheckable(true);
	actionVoronoi->setChecked(true);
	connect(actionVoronoi, &QAction::toggled, [this](bool b) { m_view->SetCalculateVoronoi(b); });
	menuCalc->addAction(actionVoronoi);

	QMenuBar *menuBar = new QMenuBar{this};
	menuBar->addMenu(menuFile);
	menuBar->addMenu(menuCalc);
	setMenuBar(menuBar);
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
