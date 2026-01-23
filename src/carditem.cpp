#include "carditem.h"

#include <QPainter>

//==========================================================
//==========================================================

CardItem::CardItem(int id, int value, const QRectF &rect)
    : QGraphicsObject()
    , m_id(id)
    , m_value(value)
    , m_rect(rect)
    , m_state(StateFaceDown)
    , m_type(TypeNumber)
    , m_borderColor(Qt::black)
    , m_backgroundColor(Qt::white)
    , m_textColor(Qt::black)
    , m_style(0)
    , m_cacheValid(false)
{
    // Enable hover events
    setAcceptHoverEvents(true);

    // Set default flags
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);

    // Set initial transform origin to center for flip animation
    setTransformOriginPoint(m_rect.center());

    // Create default graphics
    createDefaultGraphics();
}

//==========================================================

CardItem::~CardItem()
{
}

//==========================================================

void CardItem::createDefaultGraphics()
{
    // Create default back based on type
    createBackPattern();

    // Create default face based on value
    createFacePattern();

    // Default text based on value
    QStringList animalNames = {"Cat", "Dog", "Bird", "Fish", "Rabbit", "Fox", "Bear", "Lion"};
    QStringList symbolNames = {"Star", "Moon", "Sun", "Heart", "Diamond", "Club", "Spade", "Circle"};

    switch (m_type) {
    case TypeAnimal:
        m_cardText = animalNames.value(m_value % animalNames.size(), "Card");
        break;
    case TypeNumber:
        m_cardText = QString::number(m_value + 1);
        break;
    case TypeSymbol:
        m_cardText = symbolNames.value(m_value % symbolNames.size(), "Symbol");
        break;
    case TypeCustom:
        m_cardText = QString("Card %1").arg(m_id + 1);
        break;
    }
}

//==========================================================

void CardItem::createBackPattern()
{
    m_cardBack = QPixmap(m_rect.size().toSize());
    m_cardBack.fill(Qt::transparent);

    QPainter painter(&m_cardBack);
    painter.setRenderHint(QPainter::Antialiasing);
    drawCardBack(&painter);
}

//==========================================================

void CardItem::createFacePattern()
{
    m_cardFace = QPixmap(m_rect.size().toSize());
    m_cardFace.fill(Qt::transparent);

    QPainter painter(&m_cardFace);
    painter.setRenderHint(QPainter::Antialiasing);
    drawCardFace(&painter);
}

//==========================================================

void CardItem::drawCardBack(QPainter *painter)
{
    switch (m_style)
    {
    case 0: drawClassicCard(painter); break;
    //case 1: drawModernCard(painter); break;
    //case 2: drawMinimalCard(painter); break;
    //default: drawClassicCard(painter); break;
    }
}

//==========================================================

void CardItem::drawCardFace(QPainter *painter)
{
    // Draw background
    QPainterPath path;
    path.addRect(QRectF(QPointF(0, 0), m_rect.size()));

    // Fill with gradient or color
    if (m_gradient.start() != m_gradient.finalStop()) {
        painter->fillPath(path, m_gradient);
    } else {
        painter->fillPath(path, m_backgroundColor);
    }

    // Draw border
    painter->setPen(QPen(m_borderColor, 2));
    painter->drawPath(path);

    // Draw face image if available
    if (!m_cardFace.isNull()) {
        painter->drawPixmap(5, 5, m_rect.width() - 10, m_rect.height() - 10, m_cardFace);
    }

    // Draw text if available
    if (!m_cardText.isEmpty()) {
        painter->setPen(m_textColor);
        QFont font = painter->font();
        font.setPointSize(16);
        font.setBold(true);
        painter->setFont(font);

        painter->drawText(QRectF(10, 10, m_rect.width() - 20, m_rect.height() - 20),
                          Qt::AlignCenter, m_cardText);
    }

    // Draw value indicator in corners
    painter->setPen(m_textColor);
    QFont smallFont = painter->font();
    smallFont.setPointSize(10);
    painter->setFont(smallFont);

    QString valueStr = QString::number(m_value);
    painter->drawText(QRectF(5, 5, 20, 20), Qt::AlignCenter, valueStr);
    painter->drawText(QRectF(m_rect.width() - 25, m_rect.height() - 25, 20, 20),
                      Qt::AlignCenter, valueStr);
}

//==========================================================

void CardItem::drawClassicCard(QPainter *painter)
{
    QPainterPath path;
    path.addRect(QRectF(QPointF(0, 0), m_rect.size()));

    // Classic blue gradient back
    QLinearGradient grad(0, 0, m_rect.width(), m_rect.height());
    grad.setColorAt(0, QColor("#3498db"));
    grad.setColorAt(1, QColor("#2980b9"));
    painter->fillPath(path, grad);

    // Border
    painter->setPen(QPen(QColor("#2c3e50"), 2));
    painter->drawPath(path);

    // Classic pattern
    painter->setPen(QPen(QColor("#ecf0f1"), 2));
    painter->setBrush(Qt::NoBrush);

    // Draw diamond pattern
    QPointF center(m_rect.width() / 2, m_rect.height() / 2);
    qreal size = qMin(m_rect.width(), m_rect.height()) * 0.3;

    QPolygonF diamond;
    diamond << center + QPointF(0, -size)
            << center + QPointF(size, 0)
            << center + QPointF(0, size)
            << center + QPointF(-size, 0);

    painter->drawPolygon(diamond);

    // Draw smaller diamonds in corners
    qreal smallSize = size * 0.4;
    QPointF corners[4] = {
        QPointF(20, 20),
        QPointF(m_rect.width() - 20, 20),
        QPointF(20, m_rect.height() - 20),
        QPointF(m_rect.width() - 20, m_rect.height() - 20)
    };

    for (const QPointF &corner : corners) {
        QPolygonF smallDiamond;
        smallDiamond << corner + QPointF(0, -smallSize)
                     << corner + QPointF(smallSize, 0)
                     << corner + QPointF(0, smallSize)
                     << corner + QPointF(-smallSize, 0);
        painter->drawPolygon(smallDiamond);
    }
}

//==========================================================

QRectF CardItem::boundingRect() const
{
    // Add some padding for effects
    qreal padding = 5.0;
    return m_rect.adjusted(-padding, -padding, padding, padding);
}

//==========================================================

QPainterPath CardItem::shape() const
{
    QPainterPath path;
    path.addRect(m_rect);
    return path;
}

//==========================================================

void CardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    // Use cached pixmap for better performance
    if (!m_cacheValid || m_cachedPixmap.size() != m_rect.size().toSize()) {
        m_cachedPixmap = QPixmap(m_rect.size().toSize() * painter->device()->devicePixelRatio());
        m_cachedPixmap.setDevicePixelRatio(painter->device()->devicePixelRatio());
        m_cachedPixmap.fill(Qt::transparent);

        QPainter cachePainter(&m_cachedPixmap);
        cachePainter.setRenderHint(QPainter::Antialiasing);

        // Draw card based on state
        if (m_state == StateFaceDown || m_state == StateHinted) {
            drawCardBack(&cachePainter);
        } else {
            drawCardFace(&cachePainter);
        }

        m_cacheValid = true;
    }

    // Draw cached pixmap
    painter->drawPixmap(m_rect.topLeft(), m_cachedPixmap);
}
