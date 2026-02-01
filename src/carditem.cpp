#include "carditem.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QTime>
#include <QRadialGradient>
#include <QtMath>
#include <QTimer>

//==========================================================
//==========================================================

CardItem::CardItem(int id, int value, const QRectF &rect, QObject *parent)
    : QGraphicsObject()
    , m_id(id)
    , m_value(value)
    , m_rect(rect)
    , m_state(StateFaceDown)
    , m_type(TypeNumber)
    , m_borderColor(Qt::black)
    , m_backgroundColor(Qt::white)
    , m_highlightColor(QColor("#3498db"))
    , m_textColor(Qt::black)
    , m_roundedCorners(true)
    , m_cornerRadius(10.0)
    , m_hasShadow(true)
    , m_hasGlow(false)
    , m_isHighlighted(false)
    , m_style(0)
    , m_shadowEffect(nullptr)
    , m_glowEffect(nullptr)
    , m_flipAnimation(nullptr)
    , m_scaleAnimation(nullptr)
    , m_moveAnimation(nullptr)
    , m_opacityAnimation(nullptr)
    , m_sequenceAnimation(nullptr)
    , m_parallelAnimation(nullptr)
    , m_pressed(false)
    , m_hovered(false)
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

    // Create effects
    updateEffects();

    // Create animations
    m_flipAnimation = new QPropertyAnimation(this, "rotation");
    m_flipAnimation->setDuration(300);
    m_flipAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    m_scaleAnimation = new QPropertyAnimation(this, "scale");
    m_scaleAnimation->setDuration(200);
    m_scaleAnimation->setEasingCurve(QEasingCurve::OutBack);

    m_moveAnimation = new QPropertyAnimation(this, "pos");
    m_moveAnimation->setDuration(500);
    m_moveAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    m_opacityAnimation = new QPropertyAnimation(this, "opacity");
    m_opacityAnimation->setDuration(300);
    m_opacityAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    m_sequenceAnimation = new QSequentialAnimationGroup(this);
    m_parallelAnimation = new QParallelAnimationGroup(this);

    // Connect animation signals
    connect(m_flipAnimation, &QPropertyAnimation::finished,
            this, &CardItem::onAnimationFinished);
    connect(m_sequenceAnimation, &QSequentialAnimationGroup::finished,
            this, &CardItem::onAnimationFinished);
    connect(m_parallelAnimation, &QParallelAnimationGroup::finished,
            this, &CardItem::onAnimationFinished);
}

//==========================================================

CardItem::~CardItem()
{
    if (m_flipAnimation && m_flipAnimation->state() == QAbstractAnimation::Running)
        m_flipAnimation->stop();
    if (m_sequenceAnimation && m_sequenceAnimation->state() == QAbstractAnimation::Running)
        m_sequenceAnimation->stop();
    if (m_parallelAnimation && m_parallelAnimation->state() == QAbstractAnimation::Running)
        m_parallelAnimation->stop();
}

//==========================================================

QRectF CardItem::boundingRect() const
{
    // Add some padding for effects
    qreal padding = m_hasGlow ? 20.0 : (m_hasShadow ? 10.0 : 5.0);
    return m_rect.adjusted(-padding, -padding, padding, padding);
}

//==========================================================

QPainterPath CardItem::shape() const
{
    QPainterPath path;
    if (m_roundedCorners) {
        path.addRoundedRect(m_rect, m_cornerRadius, m_cornerRadius);
    } else {
        path.addRect(m_rect);
    }
    return path;
}

//==========================================================

void CardItem::setCardBack(const QPixmap &pixmap)
{
    m_cardBack = pixmap.scaled(m_rect.size().toSize(),
                               Qt::KeepAspectRatioByExpanding,
                               Qt::SmoothTransformation);
    m_cacheValid = false;
    update();
}

//==========================================================

void CardItem::setCardFace(const QPixmap &pixmap)
{
    m_cardFace = pixmap.scaled(m_rect.size().toSize(),
                               Qt::KeepAspectRatioByExpanding,
                               Qt::SmoothTransformation);
    m_cacheValid = false;
    update();
}

//==========================================================

void CardItem::setCardText(const QString &text)
{
    m_cardText = text;
    m_cacheValid = false;
    update();
}

//==========================================================

void CardItem::setCardType(CardType type)
{
    m_type = type;
    createDefaultGraphics();
    m_cacheValid = false;
    update();
}

//==========================================================

void CardItem::setFlipped(bool flipped, bool animated)
{
    CardState newState = flipped ? StateFaceUp : StateFaceDown;

    if (m_state == newState) return;

    if (animated) {
        flipAnimation(flipped);
    } else {
        m_state = newState;
        m_cacheValid = false;
        update();
        //emit flipped(this, flipped); //???
    }
}

//==========================================================

void CardItem::setMatched(bool matched, bool animated)
{
    CardState newState = matched ? StateMatched : StateFaceDown;

    if (m_state == StateMatched && !matched) {
        // Can't un-match a card
        return;
    }

    m_state = newState;

    if (animated && matched) {
        matchAnimation();
    } else {
        m_cacheValid = false;
        update();
        if (matched) {
            //emit matched(this); //???
        }
    }
}

//==========================================================

void CardItem::setHinted(bool hinted, bool animated)
{
    CardState newState = hinted ? StateHinted : StateFaceDown;

    if (m_state == newState) return;

    if (hinted) {
        m_state = StateHinted;
        setGlowEffect(true, QColor("#f1c40f"));
    } else {
        m_state = StateFaceDown;
        setGlowEffect(false);
    }

    if (animated) {
        bounceAnimation();
    }

    m_cacheValid = false;
    update();
}

//==========================================================

void CardItem::setEnabled(bool enabled)
{
    m_state = enabled ? StateFaceDown : StateDisabled;
    setFlag(QGraphicsItem::ItemIsSelectable, enabled);
    setAcceptHoverEvents(enabled);
    setOpacity(enabled ? 1.0 : 0.5);
    m_cacheValid = false;
    update();
}

//==========================================================

void CardItem::setGlowEffect(bool enabled, const QColor &color)
{
    m_hasGlow = enabled;

    if (enabled && !m_glowEffect) {
        m_glowEffect = new QGraphicsDropShadowEffect();
        m_glowEffect->setOffset(0, 0);
        m_glowEffect->setBlurRadius(20);
        setGraphicsEffect(m_glowEffect);
    }

    if (m_glowEffect) {
        m_glowEffect->setEnabled(enabled);
        if (enabled) {
            m_glowEffect->setColor(color);
        }
    }

    m_cacheValid = false;
    prepareGeometryChange();
    update();
}

//==========================================================

void CardItem::setShadowEffect(bool enabled, const QColor &color)
{
    m_hasShadow = enabled;

    if (enabled && !m_shadowEffect) {
        m_shadowEffect = new QGraphicsDropShadowEffect();
        m_shadowEffect->setOffset(5, 5);
        m_shadowEffect->setBlurRadius(15);
        setGraphicsEffect(m_shadowEffect);
    }

    if (m_shadowEffect) {
        m_shadowEffect->setEnabled(enabled);
        if (enabled) {
            m_shadowEffect->setColor(color);
        }
    }

    m_cacheValid = false;
    update();
}

//==========================================================

void CardItem::setBorderColor(const QColor &color)
{
    m_borderColor = color;
    m_cacheValid = false;
    update();
}

//==========================================================

void CardItem::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
    m_cacheValid = false;
    update();
}

//==========================================================

void CardItem::setHighlighted(bool highlighted)
{
    m_isHighlighted = highlighted;
    m_cacheValid = false;
    update();
}

//==========================================================

void CardItem::flipAnimation(bool toFaceUp)
{
    if (m_flipAnimation->state() == QAbstractAnimation::Running) {
        m_flipAnimation->stop();
    }

    m_flipAnimation->setStartValue(toFaceUp ? 0 : 180);
    m_flipAnimation->setEndValue(toFaceUp ? 180 : 0);
    m_flipAnimation->start();

    // Update state halfway through animation
    QTimer::singleShot(m_flipAnimation->duration() / 2, [this, toFaceUp]() {
        m_state = toFaceUp ? StateFaceUp : StateFaceDown;
        m_cacheValid = false;
        update();
        emit flipped(this, toFaceUp);
    });
}

//==========================================================

void CardItem::matchAnimation()
{
    if (m_sequenceAnimation->state() == QAbstractAnimation::Running) {
        m_sequenceAnimation->stop();
    }

    m_sequenceAnimation->clear();

    // Bounce up
    QPropertyAnimation *bounceUp = new QPropertyAnimation(this, "scale");
    bounceUp->setDuration(200);
    bounceUp->setStartValue(1.0);
    bounceUp->setEndValue(1.2);
    bounceUp->setEasingCurve(QEasingCurve::OutBack);

    // Bounce down
    QPropertyAnimation *bounceDown = new QPropertyAnimation(this, "scale");
    bounceDown->setDuration(200);
    bounceDown->setStartValue(1.2);
    bounceDown->setEndValue(1.0);
    bounceDown->setEasingCurve(QEasingCurve::InBack);

    // Fade out slightly
    QPropertyAnimation *fade = new QPropertyAnimation(this, "opacity");
    fade->setDuration(400);
    fade->setStartValue(1.0);
    fade->setEndValue(0.8);

    m_sequenceAnimation->addAnimation(bounceUp);
    m_sequenceAnimation->addAnimation(bounceDown);
    m_sequenceAnimation->addAnimation(fade);

    m_sequenceAnimation->start();

    // Add glow effect for matched cards
    setGlowEffect(true, QColor("#2ecc71"));

    emit matched(this);
}

//==========================================================

void CardItem::wrongMatchAnimation()
{
    if (m_sequenceAnimation->state() == QAbstractAnimation::Running) {
        m_sequenceAnimation->stop();
    }

    m_sequenceAnimation->clear();

    // Shake animation
    QPropertyAnimation *shake1 = new QPropertyAnimation(this, "rotation");
    shake1->setDuration(100);
    shake1->setStartValue(0);
    shake1->setEndValue(-10);

    QPropertyAnimation *shake2 = new QPropertyAnimation(this, "rotation");
    shake2->setDuration(100);
    shake2->setStartValue(-10);
    shake2->setEndValue(10);

    QPropertyAnimation *shake3 = new QPropertyAnimation(this, "rotation");
    shake3->setDuration(100);
    shake3->setStartValue(10);
    shake3->setEndValue(0);

    // Red flash
    QPropertyAnimation *colorFlash = new QPropertyAnimation(this, "opacity");
    colorFlash->setDuration(300);
    colorFlash->setStartValue(1.0);
    colorFlash->setEndValue(0.7);
    colorFlash->setEasingCurve(QEasingCurve::InOutQuad);

    m_sequenceAnimation->addAnimation(shake1);
    m_sequenceAnimation->addAnimation(shake2);
    m_sequenceAnimation->addAnimation(shake3);
    m_sequenceAnimation->addAnimation(colorFlash);

    m_sequenceAnimation->start();

    // Temporary red glow
    setGlowEffect(true, QColor("#e74c3c"));
    QTimer::singleShot(500, [this]() {
        setGlowEffect(false);
    });
}

//==========================================================

void CardItem::bounceAnimation()
{
    if (m_scaleAnimation->state() == QAbstractAnimation::Running) {
        m_scaleAnimation->stop();
    }

    m_scaleAnimation->setStartValue(1.0);
    m_scaleAnimation->setKeyValueAt(0.5, 1.1);
    m_scaleAnimation->setEndValue(1.0);
    m_scaleAnimation->start();
}

//==========================================================

void CardItem::shakeAnimation()
{
    if (m_sequenceAnimation->state() == QAbstractAnimation::Running) {
        m_sequenceAnimation->stop();
    }

    m_sequenceAnimation->clear();

    for (int i = 0; i < 3; i++) {
        QPropertyAnimation *shake = new QPropertyAnimation(this, "pos");
        shake->setDuration(50);

        if (i % 2 == 0) {
            shake->setStartValue(pos());
            shake->setEndValue(pos() + QPointF(5, 0));
        } else {
            shake->setStartValue(pos() + QPointF(5, 0));
            shake->setEndValue(pos());
        }

        m_sequenceAnimation->addAnimation(shake);
    }

    m_sequenceAnimation->start();
}

//==========================================================

void CardItem::pulsateAnimation()
{
    if (m_parallelAnimation->state() == QAbstractAnimation::Running) {
        m_parallelAnimation->stop();
    }

    m_parallelAnimation->clear();

    // Scale pulse
    QPropertyAnimation *scaleAnim = new QPropertyAnimation(this, "scale");
    scaleAnim->setDuration(1000);
    scaleAnim->setStartValue(1.0);
    scaleAnim->setKeyValueAt(0.5, 1.05);
    scaleAnim->setEndValue(1.0);
    scaleAnim->setEasingCurve(QEasingCurve::InOutSine);
    scaleAnim->setLoopCount(-1); // Infinite loop

    // Opacity pulse
    QPropertyAnimation *opacityAnim = new QPropertyAnimation(this, "opacity");
    opacityAnim->setDuration(1000);
    opacityAnim->setStartValue(1.0);
    opacityAnim->setKeyValueAt(0.5, 0.8);
    opacityAnim->setEndValue(1.0);
    opacityAnim->setEasingCurve(QEasingCurve::InOutSine);
    opacityAnim->setLoopCount(-1);

    m_parallelAnimation->addAnimation(scaleAnim);
    m_parallelAnimation->addAnimation(opacityAnim);

    m_parallelAnimation->start();
}

//==========================================================

void CardItem::moveToPosition(const QPointF &pos, int duration)
{
    if (m_moveAnimation->state() == QAbstractAnimation::Running) {
        m_moveAnimation->stop();
    }

    m_moveAnimation->setDuration(duration);
    m_moveAnimation->setStartValue(this->pos());
    m_moveAnimation->setEndValue(pos);
    m_moveAnimation->start();
}

//==========================================================

void CardItem::resetTransformations()
{
    setRotation(0);
    setScale(1.0);
    setOpacity(1.0);
}

//==========================================================

void CardItem::setRoundedCorners(bool enabled, qreal radius)
{
    m_roundedCorners = enabled;
    m_cornerRadius = radius;
    m_cacheValid = false;
    update();
}

//==========================================================

void CardItem::setGradientBackground(const QColor &startColor, const QColor &endColor)
{
    m_gradient = QLinearGradient(m_rect.topLeft(), m_rect.bottomRight());
    m_gradient.setColorAt(0, startColor);
    m_gradient.setColorAt(1, endColor);
    m_cacheValid = false;
    update();
}

//==========================================================

void CardItem::setPatternBackground(const QPixmap &pattern)
{
    m_pattern = pattern;
    m_cacheValid = false;
    update();
}

//==========================================================

void CardItem::setCardStyle(int style)
{
    m_style = style;
    m_cacheValid = false;
    update();
}

//==========================================================

void CardItem::onAnimationFinished()
{
    emit animationFinished(this);
}

//==========================================================

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_state == StateDisabled || m_state == StateMatched) {
        event->ignore();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        m_pressPos = event->pos();
        m_pressTime = QTime::currentTime();

        // Visual feedback for press
        setScale(0.95);

        event->accept();
    } else {
        event->ignore();
    }
}

//==========================================================

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_pressed || event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    m_pressed = false;

    // Reset scale
    setScale(1.0);

    // Check if this was a click (not a drag)
    QPointF releasePos = event->pos();
    qreal distance = QLineF(m_pressPos, releasePos).length();
    int elapsed = m_pressTime.msecsTo(QTime::currentTime());

    if (distance < 5 && elapsed < 500) {
        emit clicked(this);

        // Visual feedback
        bounceAnimation();
    }

    event->accept();
}

//==========================================================

void CardItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    // Double-click could be used for quick flip or other action
}

//==========================================================

void CardItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_state == StateDisabled || m_state == StateMatched) {
        event->ignore();
        return;
    }

    m_hovered = true;

    // Visual feedback for hover
    setScale(1.05);
    setZValue(10); // Bring to front

    // Add shadow on hover
    setShadowEffect(true, QColor(0, 0, 0, 100));

    emit hoverEntered(this);

    event->accept();
}

//==========================================================

void CardItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_hovered = false;

    // Reset visual effects
    setScale(1.0);
    setZValue(0);

    // Only keep shadow if card is face up or matched
    if (m_state != StateFaceUp && m_state != StateMatched) {
        setShadowEffect(false);
    }

    emit hoverLeft(this);

    event->accept();
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

    // Draw selection/highlight outline if needed
    if (m_isHighlighted || isSelected()) {
        painter->setPen(QPen(m_highlightColor, 3));
        painter->setBrush(Qt::NoBrush);

        if (m_roundedCorners) {
            painter->drawRoundedRect(m_rect.adjusted(2, 2, -2, -2),
                                     m_cornerRadius, m_cornerRadius);
        } else {
            painter->drawRect(m_rect.adjusted(2, 2, -2, -2));
        }
    }
}

//==========================================================

void CardItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;

    QAction *flipAction = menu.addAction(isFlipped() ? "Flip Down" : "Flip Up");
    QAction *matchAction = menu.addAction("Mark as Matched");
    QAction *hintAction = menu.addAction("Show Hint");
    menu.addSeparator();
    QAction *propertiesAction = menu.addAction("Properties");

    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == flipAction) {
        setFlipped(!isFlipped(), true);
    } else if (selectedAction == matchAction) {
        setMatched(true, true);
    } else if (selectedAction == hintAction) {
        setHinted(true, true);
    } else if (selectedAction == propertiesAction) {
        qDebug() << "Card ID:" << m_id << "Value:" << m_value << "State:" << m_state;
    }

    event->accept();
}

//==========================================================

void CardItem::drawCardBack(QPainter *painter)
{
    switch (m_style) {
    case 0: drawClassicCard(painter); break;
    case 1: drawModernCard(painter); break;
    case 2: drawMinimalCard(painter); break;
    default: drawClassicCard(painter); break;
    }
}

//==========================================================

void CardItem::drawCardFace(QPainter *painter)
{
    // Draw background
    QPainterPath path;
    if (m_roundedCorners) {
        path.addRoundedRect(QRectF(QPointF(0, 0), m_rect.size()),
                            m_cornerRadius, m_cornerRadius);
    } else {
        path.addRect(QRectF(QPointF(0, 0), m_rect.size()));
    }

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
    if (m_roundedCorners) {
        path.addRoundedRect(QRectF(QPointF(0, 0), m_rect.size()),
                            m_cornerRadius, m_cornerRadius);
    } else {
        path.addRect(QRectF(QPointF(0, 0), m_rect.size()));
    }

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

void CardItem::drawModernCard(QPainter *painter)
{
    QPainterPath path;
    if (m_roundedCorners) {
        path.addRoundedRect(QRectF(QPointF(0, 0), m_rect.size()),
                            m_cornerRadius, m_cornerRadius);
    } else {
        path.addRect(QRectF(QPointF(0, 0), m_rect.size()));
    }

    // Modern gradient
    QLinearGradient grad(0, 0, m_rect.width(), 0);
    grad.setColorAt(0, QColor("#9b59b6"));
    grad.setColorAt(0.5, QColor("#8e44ad"));
    grad.setColorAt(1, QColor("#9b59b6"));
    painter->fillPath(path, grad);

    // Modern border
    painter->setPen(QPen(QColor("#8e44ad"), 3));
    painter->drawPath(path);

    // Geometric pattern
    painter->setPen(QPen(QColor("#ecf0f1"), 1));

    // Draw grid pattern
    int gridSize = 20;
    for (int x = gridSize; x < m_rect.width(); x += gridSize) {
        painter->drawLine(x, 0, x, m_rect.height());
    }
    for (int y = gridSize; y < m_rect.height(); y += gridSize) {
        painter->drawLine(0, y, m_rect.width(), y);
    }

    // Draw circles at intersections
    painter->setBrush(QColor("#ecf0f1"));
    for (int x = gridSize; x < m_rect.width(); x += gridSize) {
        for (int y = gridSize; y < m_rect.height(); y += gridSize) {
            painter->drawEllipse(QPointF(x, y), 2, 2);
        }
    }
}

//==========================================================

void CardItem::drawMinimalCard(QPainter *painter)
{
    QPainterPath path;
    if (m_roundedCorners) {
        path.addRoundedRect(QRectF(QPointF(0, 0), m_rect.size()),
                            m_cornerRadius, m_cornerRadius);
    } else {
        path.addRect(QRectF(QPointF(0, 0), m_rect.size()));
    }

    // Minimal white background
    painter->fillPath(path, Qt::white);

    // Thin border
    painter->setPen(QPen(Qt::lightGray, 1));
    painter->drawPath(path);

    // Simple X pattern
    painter->setPen(QPen(Qt::lightGray, 1));
    painter->drawLine(0, 0, m_rect.width(), m_rect.height());
    painter->drawLine(m_rect.width(), 0, 0, m_rect.height());

    // Card ID in center (minimal)
    painter->setPen(Qt::darkGray);
    QFont font = painter->font();
    font.setPointSize(12);
    painter->setFont(font);

    painter->drawText(QRectF(0, 0, m_rect.width(), m_rect.height()),
                      Qt::AlignCenter, QString::number(m_id));
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

void CardItem::updateEffects()
{
    setShadowEffect(m_hasShadow);
    setGlowEffect(m_hasGlow);
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

void CardItem::updateAppearance()
{
    m_cacheValid = false;
    update();
    update(boundingRect());
}


