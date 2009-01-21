/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|						   (c) SuSE Linux GmbH |
\----------------------------------------------------------------------/

  File:		QGraph.cc

  Author:	Arvin Schnell <aschnell@suse.de>

  Textdomain	"qt-graph"

/-*/


#include <math.h>

#include "QGraph.h"


QGraph::QGraph(QWidget* parent, const std::string& filename, const std::string& layoutAlgorithm)
    : QGraphicsView(parent)
{
    init();

    renderGraph(filename, layoutAlgorithm);
}


QGraph::QGraph(QWidget* parent, graph_t* graph)
    : QGraphicsView(parent)
{
    init();

    renderGraph(graph);
}


QGraph::~QGraph()
{
}


void
QGraph::init()
{
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setResizeAnchor(AnchorUnderMouse);

    scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);
    setScene(scene);
}


void
QGraph::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Plus:
            scaleView(1.2);
            break;
        case Qt::Key_Minus:
            scaleView(1.0 / 1.2);
            break;
        default:
            QGraphicsView::keyPressEvent(event);
    }
}


void
QGraph::wheelEvent(QWheelEvent* event)
{
    scaleView(pow(2.0, -event->delta() / 240.0));
}


void
QGraph::scaleView(qreal scaleFactor)
{
    qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.05 || factor > 10)
	return;

    scale(scaleFactor, scaleFactor);
}


QPointF
QGraph::gToQ(const point& p, bool trans) const
{
    QPointF tmp(p.x, p.y);
    return trans ? QPointF(tmp.x(), size.height() - tmp.y()) : QPointF(tmp.x(), -tmp.y());
}


QPointF
QGraph::gToQ(const pointf& p, bool trans) const
{
    QPointF tmp(p.x, p.y);
    return trans ? QPointF(tmp.x(), size.height() - tmp.y()) : QPointF(tmp.x(), -tmp.y());
}


QString
aggetToQ(void* obj, char* name, const char* fallback)
{
    const char* tmp = agget(obj, name);
    if (tmp == NULL || tmp[0] == '\0')
	return fallback;
    else
	return QString(tmp);
}


void
QGraph::arrow(QPainterPath& path, const QLineF& line, const QString& type)
{
    if (type == "normal")
    {
	QLineF n(line.normalVector());
	QPointF o(n.dx() / 2.0, n.dy() / 2.0);

	path.moveTo(line.p1());
	path.moveTo(line.p1() + o);
	path.lineTo(line.p2());
	path.lineTo(line.p1() - o);
	path.closeSubpath();
    }
    else if (type == "empty")
    {
	QLineF n(line.normalVector());
	QPointF o(n.dx() / 2.0, n.dy() / 2.0);

	path.moveTo(line.p1());
	path.moveTo(line.p1() + o);
	path.lineTo(line.p2());
	path.lineTo(line.p1() - o);
	path.closeSubpath();
    }
    else if (type == "open")
    {
	QLineF n(line.normalVector());
	QPointF o(n.dx() / 2.0, n.dy() / 2.0);

	path.moveTo(line.p1());
	path.lineTo(line.p2());

	path.moveTo(line.p1() + o);
	path.lineTo(line.p2());
	path.lineTo(line.p1() - o);
    }
    else if (type == "none")
    {
	path.moveTo(line.p1());
	path.lineTo(line.p2());
    }
    else
    {
	// yuiError() << "unknown arrow type " << type.toStdString() << endl;
    }
}


void
QGraph::renderGraph(const std::string& filename, const std::string& layoutAlgorithm)
{
    GVC_t* gvc = gvContext();

    FILE* fp = fopen(filename.c_str(), "r");
    if (fp)
    {
	graph_t* graph = agread(fp);

	char* tmp = strdup(layoutAlgorithm.c_str());
	gvLayout(gvc, graph, tmp);
	free(tmp);

	renderGraph(graph);

	gvFreeLayout(gvc, graph);
	agclose(graph);
    }
    else
    {
	// yuiError() << "failed to open " << filename << endl;
    }

    gvFreeContext(gvc);
}


QPolygonF
QGraph::haha1(node_t* node) const
{
    const polygon_t* poly = (polygon_t*) ND_shape_info(node);

    const int sides = poly->sides;
    const pointf* vertices = poly->vertices;

    QPolygonF polygon;
    for (int side = 0; side < sides; side++)
	polygon.append(gToQ(vertices[side], false));
    return polygon;
}



QPainterPath
QGraph::haha2(node_t* node) const
{
    QPainterPath path;

    const char* name = ND_shape(node)->name;

    if ((strcmp(name, "rectangle") == 0) ||
	(strcmp(name, "diamond") == 0) ||
	(strcmp(name, "triangle") == 0))
    {
	QPolygonF polygon = haha1(node);
	polygon.append(polygon[0]);
	path.addPolygon(polygon);
    }

    if ((strcmp(name, "ellipse") == 0) ||
	(strcmp(name, "circle") == 0))
    {
	QPolygonF polygon = haha1(node);
	path.addEllipse(QRectF(polygon[0], polygon[1]));
    }

    return path;
}


void
QGraph::drawLabel(const textlabel_t* textlabel, QPainter* painter)
{
    painter->setPen(textlabel->fontcolor);

    QRectF rect(-100, -100, 100, 100); // TODO
    rect.moveCenter(gToQ(textlabel->p, false));
    painter->drawText(rect, Qt::AlignCenter | Qt::AlignHCenter, textlabel->text);
}


void
QGraph::renderGraph(graph_t* graph)
{
    // don't use gToQ here since it adjusts the values
    QRectF rect(GD_bb(graph).LL.x, GD_bb(graph).LL.y, GD_bb(graph).UR.x, GD_bb(graph).UR.y);
    const qreal border = 20.0;
    scene->setSceneRect(rect.adjusted(-border, -border, border, border));

    size = rect.size();


    QPen pen1("#0000ff");
    pen1.setWidthF(1);
    QBrush brush1("#bbbbff");

    QPen pen2(Qt::black);
    pen2.setWidthF(1);


    for (node_t* node = agfstnode(graph); node != NULL; node = agnxtnode(graph, node))
    {
	Node* shape = new Node(haha2(node));

	scene->addItem(shape);

	shape->setPos(gToQ(ND_coord_i(node), true));

	shape->setPen(pen1);
	shape->setBrush(brush1);

	QPainter painter;
	painter.begin(&shape->picture);
	drawLabel(ND_label(node), &painter);
	painter.end();

	const char* tooltip = agget(node, "tooltip");
	if (tooltip && strlen(tooltip) > 0)
	    shape->setToolTip(tooltip);

	for (edge_t* edge = agfstout(graph, node); edge != NULL; edge = agnxtout(graph, edge))
	{
	    const splines* spl = ED_spl(edge);

	    for (int i = 0; i < spl->size; i++)
	    {
		const bezier& bz = spl->list[i];

		QPainterPath path;

		if (bz.sflag)
		    arrow(path, QLineF(gToQ(bz.list[0], true), gToQ(bz.sp, true)), aggetToQ(edge, "arrowhead", "normal"));

		path.moveTo(gToQ(bz.list[0], true));
		for (int j = 1; j < bz.size-1; j += 3)
		    path.cubicTo(gToQ(bz.list[j], true), gToQ(bz.list[j+1], true), gToQ(bz.list[j+2], true));

		if (bz.eflag)
		    arrow(path, QLineF(gToQ(bz.list[bz.size-1], true), gToQ(bz.ep, true)), aggetToQ(edge, "arrowtail", "normal"));

		QGraphicsPathItem* shape = scene->addPath(path);

		shape->setPen(pen2);
		shape->setZValue(-1.0);
	    }
	}
    }
}


Node::Node(const QPainterPath& path)
    : QGraphicsPathItem(path)
{
    // setFlag(ItemIsMovable);
}


void
Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // TODO: rethink painter state handling, see also QGraphicsView::DontSavePainterState

    painter->save();
    QGraphicsPathItem::paint(painter, option, widget);
    painter->restore();

    picture.play(painter);
}


#include "QGraph.moc"
