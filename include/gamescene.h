#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>

//==========================================================
//==========================================================

class GameScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit GameScene(QObject *parent = nullptr);
    ~GameScene();

private:
    // Visual effects
    QColor m_backgroundColor;
    QColor m_gridColor;
    QBrush m_backgroundBrush;
    QPixmap m_backgroundPattern;
};

//==========================================================
//==========================================================

#endif // GAMESCENE_H
