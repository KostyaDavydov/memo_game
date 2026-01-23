#ifndef CARDITEM_H
#define CARDITEM_H

#include <QGraphicsObject>
#include <QLinearGradient>

//==========================================================
//==========================================================

class CardItem : public QGraphicsObject
{
    Q_OBJECT

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

    CardItem(int id, int value, const QRectF &rect);
    virtual ~CardItem();

    // Getters
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    // Help methods
    void createDefaultGraphics();
    void createBackPattern();
    void createFacePattern();
    void drawCardBack(QPainter *painter);
    void drawCardFace(QPainter *painter);
    void drawClassicCard(QPainter *painter);

private:
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
    QColor m_textColor;

    QLinearGradient m_gradient;

    int m_style;

    // Cache for better performance
    QPixmap m_cachedPixmap;
    bool m_cacheValid;
};

//==========================================================
//==========================================================

#endif // CARDITEM_H
