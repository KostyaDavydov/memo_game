#include "gamescene.h"

#include <QPainter>

//==========================================================
//==========================================================

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent)
{
    // Setup scene
    setSceneRect(0, 0, 800, 600);

    // Initialize colors
    m_backgroundColor = QColor("#34495e");
    m_gridColor = QColor("#2c3e50");
    m_backgroundBrush = QBrush(m_backgroundColor);

    // Create background pattern
    m_backgroundPattern = QPixmap(64, 64);
    m_backgroundPattern.fill(Qt::transparent);
    QPainter patternPainter(&m_backgroundPattern);
    patternPainter.setPen(QPen(QColor(255, 255, 255, 10), 1));
    patternPainter.drawRect(0, 0, 63, 63);
    patternPainter.end();
}

//==========================================================

GameScene::~GameScene()
{
}
