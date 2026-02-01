#ifndef CARDITEM_H
#define CARDITEM_H

//==========================================================
//==========================================================

#include <QGraphicsObject>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QTime>

//==========================================================
//==========================================================

class CardItem : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)
    Q_PROPERTY(qreal scale READ scale WRITE setScale)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

public:
    enum CardState {
        StateFaceDown,
        StateFaceUp,
        StateMatched,
        StateHinted,
        StateDisabled
    };

    enum CardType {
        TypeAnimal,
        TypeNumber,
        TypeSymbol,
        TypeCustom
    };

    explicit CardItem(int id, int value, const QRectF &rect, QObject *parent = nullptr);
    virtual ~CardItem();

    // Getters
    int getId() const { return m_id; }
    int getCardValue() const { return m_value; }
    CardState getState() const { return m_state; }
    bool isFlipped() const { return m_state == StateFaceUp || m_state == StateMatched; }
    bool isMatched() const { return m_state == StateMatched; }
    bool isHinted() const { return m_state == StateHinted; }
    bool isEnabled() const { return m_state != StateDisabled; }
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    // Setters
    void setCardBack(const QPixmap &pixmap);
    void setCardFace(const QPixmap &pixmap);
    void setCardText(const QString &text);
    void setCardType(CardType type);
    void setFlipped(bool flipped, bool animated = true);
    void setMatched(bool matched, bool animated = true);
    void setHinted(bool hinted, bool animated = true);
    void setEnabled(bool enabled);
    void setGlowEffect(bool enabled, const QColor &color = QColor("#f1c40f"));
    void setShadowEffect(bool enabled, const QColor &color = Qt::black);
    void setBorderColor(const QColor &color);
    void setBackgroundColor(const QColor &color);
    void setHighlighted(bool highlighted);

    // Animation
    void flipAnimation(bool toFaceUp);
    void matchAnimation();
    void wrongMatchAnimation();
    void bounceAnimation();
    void shakeAnimation();
    void pulsateAnimation();
    void moveToPosition(const QPointF &pos, int duration = 500);
    void resetTransformations();

    // Visual customization
    void setRoundedCorners(bool enabled, qreal radius = 10.0);
    void setGradientBackground(const QColor &startColor, const QColor &endColor);
    void setPatternBackground(const QPixmap &pattern);
    void setCardStyle(int style); // 0: classic, 1: modern, 2: minimal

signals:
    void clicked(CardItem *card);
    void hoverEntered(CardItem *card);
    void hoverLeft(CardItem *card);
    void flipped(CardItem *card, bool faceUp);
    void matched(CardItem *card);
    void animationFinished(CardItem *card);

public slots:
    void onAnimationFinished();

protected:
    // Event handlers
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    // Helper methods
    void updateAppearance();
    void drawCardBack(QPainter *painter);
    void drawCardFace(QPainter *painter);
    void drawClassicCard(QPainter *painter);
    void drawModernCard(QPainter *painter);
    void drawMinimalCard(QPainter *painter);
    void createBackPattern();
    void createFacePattern();
    void updateEffects();
    void createDefaultGraphics();

    // Card properties
    int m_id;
    int m_value;
    QRectF m_rect;
    CardState m_state;
    CardType m_type;

    // Visual properties
    QPixmap m_cardBack;
    QPixmap m_cardFace;
    QString m_cardText;

    QColor m_borderColor;
    QColor m_backgroundColor;
    QColor m_highlightColor;
    QColor m_textColor;

    QLinearGradient m_gradient;
    QPixmap m_pattern;

    bool m_roundedCorners;
    qreal m_cornerRadius;
    bool m_hasShadow;
    bool m_hasGlow;
    bool m_isHighlighted;
    int m_style;

    // Effects
    QGraphicsDropShadowEffect *m_shadowEffect;
    QGraphicsDropShadowEffect *m_glowEffect;

    // Animation
    QPropertyAnimation *m_flipAnimation;
    QPropertyAnimation *m_scaleAnimation;
    QPropertyAnimation *m_moveAnimation;
    QPropertyAnimation *m_opacityAnimation;
    QSequentialAnimationGroup *m_sequenceAnimation;
    QParallelAnimationGroup *m_parallelAnimation;

    // Interaction state
    bool m_pressed;
    bool m_hovered;
    QPointF m_pressPos;
    QTime m_pressTime;

    // Cache for better performance
    QPixmap m_cachedPixmap;
    bool m_cacheValid;
};

//==========================================================
//==========================================================

#endif // CARDITEM_H


