/**
 * convex hull test program
 * @author Tobias Weber
 * @date 15-Aug-2020
 * @license: see 'LICENSE.GPL' file
 */

#include "hull.h"

#include <QApplication>
#include <QMenuBar>
#include <QLabel>
#include <QStatusBar>
#include <QMouseEvent>
#include <QMessageBox>

#include <locale>
#include <memory>
#include <array>
#include <vector>
#include <iostream>


using t_real = double;
using t_vec = m::vec<t_real, std::vector>;
using t_mat = m::mat<t_real, std::vector>;



// ----------------------------------------------------------------------------
// #define HULL_CHECK

#ifdef HULL_CHECK
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
#endif
// ----------------------------------------------------------------------------



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

	setBackgroundBrush(QBrush{QColor::fromRgbF(0.95, 0.95, 0.95, 1.)});
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

	QList<QGraphicsItem*> items = this->items(posVP);
	QGraphicsItem* item = nullptr;
	bool item_is_vertex = false;

	for(int itemidx=0; itemidx<items.size(); ++itemidx)
	{
		item = items[itemidx];
		item_is_vertex = m_vertices.find(static_cast<Vertex*>(item)) != m_vertices.end();
		if(item_is_vertex)
			break;
	}

	// only select vertices
	if(!item_is_vertex)
		item = nullptr;


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
			if(item_is_vertex)
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
			UpdateAll();
		}
	}

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

	QPoint posVP = evt->pos();
	QPointF posScene = mapToScene(posVP);
	emit SignalMouseCoordinates(posScene.x(), posScene.y());
}


void HullView::SetCalculateHull(bool b)
{
	m_calchull = b;
	UpdateHull();
}


void HullView::SetCalculateVoronoi(bool b)
{
	m_calcvoronoi = b;
	UpdateDelaunay();
}


void HullView::SetCalculateDelaunay(bool b)
{
	m_calcdelaunay = b;
	UpdateDelaunay();
}


void HullView::SetHullCalculationMethod(HullCalculationMethod m)
{
	m_hullcalculationmethod = m;
	UpdateHull();
}


void HullView::SetDelaunayCalculationMethod(DelaunayCalculationMethod m)
{
	m_delaunaycalculationmethod = m;
	UpdateDelaunay();
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
	UpdateDelaunay();
	UpdateHull();
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

	std::vector<t_vec> vertices;
	vertices.reserve(m_vertices.size());
	std::transform(m_vertices.begin(), m_vertices.end(), std::back_inserter(vertices),
		[](const Vertex* vert) -> t_vec { return m::create<t_vec>({vert->x(), vert->y()}); } );


	std::vector<std::vector<t_vec>> hull;

	switch(m_hullcalculationmethod)
	{
		case HullCalculationMethod::QHULL:
			std::tie(std::ignore, hull) = calc_delaunay<t_vec>(2, vertices, true);
			break;
		case HullCalculationMethod::DIVIDE:
			hull.emplace_back(calc_hull_divide<t_vec>(vertices));
			break;
		default:
			QMessageBox::critical(this, "Error", "Unknown hull calculation method.");
			break;
	}


#ifdef HULL_CHECK
	std::vector<QPointF> hullvertices;
	for(const auto& thetriag : hull)
		for(std::size_t idx1=0; idx1<thetriag.size(); ++idx1)
			hullvertices.emplace_back(QPointF{thetriag[idx1][0], thetriag[idx1][1]});
#endif

	// convex hull
	QPen penHull;
	penHull.setWidthF(2.);

	for(const auto& thetriag : hull)
	{
		for(std::size_t idx1=0; idx1<thetriag.size(); ++idx1)
		{
			std::size_t idx2 = idx1+1;
			if(idx2 >= thetriag.size())
				idx2 = 0;
			if(idx1 == idx2)
				continue;

			QLineF line{QPointF{thetriag[idx1][0], thetriag[idx1][1]}, QPointF{thetriag[idx2][0], thetriag[idx2][1]}};
#ifdef HULL_CHECK
			if(!all_points_on_same_side(line, hullvertices))
				continue;
#endif

			QGraphicsItem *item = m_scene->addLine(line, penHull);
			m_hull.insert(item);
		}
	}
}


void HullView::UpdateDelaunay()
{
	// remove previous triangulation
	for(QGraphicsItem* item : m_delaunay)
	{
		m_scene->removeItem(item);
		delete item;
	}
	m_delaunay.clear();

	// remove previous voronoi vertices
	for(QGraphicsItem* item : m_voronoi)
	{
		m_scene->removeItem(item);
		delete item;
	}
	m_voronoi.clear();


	if((!m_calcdelaunay && !m_calcvoronoi) || m_vertices.size() < 4)
		return;

	std::vector<t_vec> vertices;
	vertices.reserve(m_vertices.size());
	std::transform(m_vertices.begin(), m_vertices.end(), std::back_inserter(vertices),
		[](const Vertex* vert) -> t_vec { return m::create<t_vec>({vert->x(), vert->y()}); } );


	std::vector<t_vec> voronoi{};
	std::vector<std::vector<t_vec>> triags{};

	switch(m_delaunaycalculationmethod)
	{
		case DelaunayCalculationMethod::QHULL:
			std::tie(voronoi, triags) = calc_delaunay<t_vec>(2, vertices, false);
			break;
		case DelaunayCalculationMethod::PARABOLIC:
			std::tie(voronoi, triags) = calc_delaunay_parabolic<t_vec>(vertices);
			break;
		default:
			QMessageBox::critical(this, "Error", "Unknown Delaunay calculation method.");
			break;
	}


	const t_real itemRad = 7.;

	if(m_calcvoronoi)
	{
		// voronoi vertices
		QPen penVoronoi;
		penVoronoi.setStyle(Qt::SolidLine);
		penVoronoi.setWidthF(1.);

		QPen penCircle;
		penCircle.setStyle(Qt::DotLine);
		penCircle.setWidthF(1.);
		penCircle.setColor(QColor::fromRgbF(1.,0.,0.));

		QBrush brushVoronoi;
		brushVoronoi.setStyle(Qt::SolidPattern);
		brushVoronoi.setColor(QColor::fromRgbF(1.,0.,0.));

		for(std::size_t idx=0; idx<voronoi.size(); ++idx)
		{
			const t_vec& voronoivert = voronoi[idx];

			QPointF voronoipt{voronoivert[0], voronoivert[1]};
			QGraphicsItem *voronoiItem = m_scene->addEllipse(
				voronoipt.x()-itemRad/2., voronoipt.y()-itemRad/2., itemRad, itemRad, penVoronoi, brushVoronoi);
			m_voronoi.insert(voronoiItem);

			// circles
			if(idx < triags.size())
			{
				const auto& triag = triags[idx];
				if(triag.size() >= 3)
				{
					t_real rad = m::norm(voronoivert-triag[0]);

					QGraphicsItem *voronoiCircle = m_scene->addEllipse(
						voronoipt.x()-rad, voronoipt.y()-rad, rad*2., rad*2., penCircle);
					m_voronoi.insert(voronoiCircle);
				}
			}
		}
	}


	if(m_calcdelaunay)
	{
		// delaunay triangles
		for(const auto& thetriag : triags)
		{
			for(std::size_t idx1=0; idx1<thetriag.size(); ++idx1)
			{
				std::size_t idx2 = idx1+1;
				if(idx2 >= thetriag.size())
					idx2 = 0;

				QLineF line{QPointF{thetriag[idx1][0], thetriag[idx1][1]}, QPointF{thetriag[idx2][0], thetriag[idx2][1]}};
				QGraphicsItem *item = m_scene->addLine(line);
				m_delaunay.insert(item);
			}
		}
	}
}

// ----------------------------------------------------------------------------




// ----------------------------------------------------------------------------

HullWnd::HullWnd(QWidget* pParent) : QMainWindow{pParent},
	m_scene{new QGraphicsScene{this}},
	m_view{new HullView{m_scene.get(), this}}
{
	m_view->setRenderHints(QPainter::Antialiasing);

	setWindowTitle("Hull");
	setCentralWidget(m_view.get());

	QStatusBar *statusBar = new QStatusBar{this};
	m_statusLabel = std::make_shared<QLabel>(statusBar);
	statusBar->addPermanentWidget(m_statusLabel.get(), 1);
	setStatusBar(statusBar);


	// menu actions
	QAction *actionNew = new QAction{"New", this};
	connect(actionNew, &QAction::triggered, [this]()
		{ m_view->ClearVertices(); });

	QAction *actionQuit = new QAction{"Exit", this};
	connect(actionQuit, &QAction::triggered, [this]()
		{ this->close(); });


	QAction *actionHull = new QAction{"Convex Hull", this};
	actionHull->setCheckable(true);
	actionHull->setChecked(true);
	connect(actionHull, &QAction::toggled, [this](bool b)
		{ m_view->SetCalculateHull(b); });

	QAction *actionVoronoi = new QAction{"Voronoi Vertices", this};
	actionVoronoi->setCheckable(true);
	actionVoronoi->setChecked(true);
	connect(actionVoronoi, &QAction::toggled, [this](bool b)
		{ m_view->SetCalculateVoronoi(b); });

	QAction *actionDelaunay = new QAction{"Delaunay Triangulation", this};
	actionDelaunay->setCheckable(true);
	actionDelaunay->setChecked(true);
	connect(actionDelaunay, &QAction::toggled, [this](bool b)
		{ m_view->SetCalculateDelaunay(b); });


	QAction *actionHullQHull = new QAction{"QHull", this};
	actionHullQHull->setCheckable(true);
	actionHullQHull->setChecked(true);
	connect(actionHullQHull, &QAction::toggled, [this]()
		{ m_view->SetHullCalculationMethod(HullCalculationMethod::QHULL); });

	QAction *actionHullDivide = new QAction{"Divide && Conquer", this};
	actionHullDivide->setCheckable(true);
	connect(actionHullDivide, &QAction::toggled, [this]()
	{ m_view->SetHullCalculationMethod(HullCalculationMethod::DIVIDE); });


	QAction *actionDelaunayQHull = new QAction{"QHull", this};
	actionDelaunayQHull->setCheckable(true);
	actionDelaunayQHull->setChecked(true);
	connect(actionDelaunayQHull, &QAction::toggled, [this]()
		{ m_view->SetDelaunayCalculationMethod(DelaunayCalculationMethod::QHULL); });

	QAction *actionDelaunayPara = new QAction{"Parabolic Trafo", this};
	actionDelaunayPara->setCheckable(true);
	connect(actionDelaunayPara, &QAction::toggled, [this]()
		{ m_view->SetDelaunayCalculationMethod(DelaunayCalculationMethod::PARABOLIC); });


	QActionGroup *groupHullBack = new QActionGroup{this};
	groupHullBack->addAction(actionHullQHull);
	// TODO: unfinished...
	//groupHullBack->addAction(actionHullDivide);

	QActionGroup *groupDelaunayBack = new QActionGroup{this};
	groupDelaunayBack->addAction(actionDelaunayQHull);
	groupDelaunayBack->addAction(actionDelaunayPara);


	// menu
	QMenu *menuFile = new QMenu{"File", this};
	QMenu *menuCalc = new QMenu{"Calculate", this};
	QMenu *menuBack = new QMenu{"Backend", this};

	menuFile->addAction(actionNew);
	menuFile->addSeparator();
	menuFile->addAction(actionQuit);

	menuCalc->addAction(actionHull);
	menuCalc->addAction(actionVoronoi);
	menuCalc->addAction(actionDelaunay);

	menuBack->addSeparator()->setText("Convex Hull");
	menuBack->addAction(actionHullQHull);
	// TODO: unfinished...
	//menuBack->addAction(actionHullDivide);
	menuBack->addSeparator()->setText("Delaunay");
	menuBack->addAction(actionDelaunayQHull);
	menuBack->addAction(actionDelaunayPara);


	// menu bar
	QMenuBar *menuBar = new QMenuBar{this};
	menuBar->addMenu(menuFile);
	menuBar->addMenu(menuCalc);
	menuBar->addMenu(menuBack);
	setMenuBar(menuBar);


	// connections
	connect(m_view.get(), &HullView::SignalMouseCoordinates, [this](double x, double y) -> void
	{
		SetStatusMessage(QString("x=%1, y=%2.").arg(x, 5).arg(y, 5));
	});


	SetStatusMessage("Ready.");
}


void HullWnd::SetStatusMessage(const QString& msg)
{
	m_statusLabel->setText(msg);
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
	try
	{
		auto app = std::make_unique<QApplication>(argc, argv);
		set_locales();

		auto hullwnd = std::make_unique<HullWnd>();
		hullwnd->resize(1024, 768);
		hullwnd->show();

		return app->exec();
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}

	return -1;
}
// ----------------------------------------------------------------------------
