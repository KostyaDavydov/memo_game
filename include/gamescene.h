#ifndef GAMESCENE_H
#define GAMESCENE_H

//==========================================================
//==========================================================

#include <QGraphicsScene>
#include <QTimer>
#include <QPropertyAnimation>
#include <QMap>

//==========================================================
//==========================================================

class CardItem;

class GameScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit GameScene(QObject *parent = nullptr);
    ~GameScene();

    // Scene setup methods
    void setupScene(int cardCount, int difficulty);
    void clearScene();
    void resetCards();

    // Card management
    QList<CardItem*> getCards() const;
    CardItem* getCardAt(const QPointF &pos) const;
    QList<CardItem*> getSelectedCards() const;

    // State management
    bool isGameActive() const { return m_gameActive; }
    int getScore() const { return m_score; }
    int getMoves() const { return m_moves; }
    int getMatches() const { return m_matches; }

    // Grid configuration
    void setGridSize(int columns, int rows);
    QSize getGridSize() const { return QSize(m_columns, m_rows); }
    QRectF getGridBounds() const;

public slots:
    void startGame();
    void pauseGame(bool paused);
    void endGame();
    void shuffleCards();
    void checkForMatch();
    void processCardClick(CardItem* card);

signals:
    void gameStarted();
    void gamePaused(bool paused);
    void gameEnded(int score, int moves, int time);
    void scoreChanged(int score);
    void movesChanged(int moves);
    void matchesChanged(int matches);
    void cardFlipped(CardItem* card);
    void matchFound(CardItem* card1, CardItem* card2);
    void noMatchFound(CardItem* card1, CardItem* card2);
    void hintRequested();
    void gameStateChanged(const QString &state);

protected:
    // Event handlers
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    //void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    //void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private slots:
    void onFlipAnimationFinished();
    void onMatchCheckTimeout();
    void onHintTimeout();
    void updateTimer();

private:
    // Helper methods
    void initializeCards();
    void layoutCards();
    void calculateGrid(int cardCount);
    void updateGameState(const QString &state);
    void addScore(int points);
    void incrementMoves();
    void incrementMatches();
    void flipAllCards(bool faceUp, bool animated = true);
    void flipCard(CardItem* card, bool faceUp, bool animated = true);
    void removeMatchedCards();
    void highlightCard(CardItem* card, bool highlight = true);
    void showHint();
    QPixmap createCardBack(int type = 0) const;
    QPixmap createCardFace(int value, int suit) const;
    QPointF calculateCardPosition(int index) const;

    // Game state
    bool m_gameActive;
    bool m_gamePaused;
    bool m_waitingForCheck;

    int m_score;
    int m_moves;
    int m_matches;
    int m_totalPairs;
    int m_hintsRemaining;

    QTimer *m_gameTimer;
    QTimer *m_matchCheckTimer;
    QTimer *m_hintTimer;
    int m_elapsedTime; // in seconds

    // Grid configuration
    int m_columns;
    int m_rows;
    int m_cardWidth;
    int m_cardHeight;
    int m_cardSpacing;

    // Card tracking
    QList<CardItem*> m_cards;
    QList<CardItem*> m_flippedCards;
    QList<CardItem*> m_matchedCards;
    CardItem* m_lastClickedCard;
    CardItem* m_hintCard1;
    CardItem* m_hintCard2;

    // Visual effects
    QColor m_backgroundColor;
    QColor m_gridColor;
    QBrush m_backgroundBrush;
    QPixmap m_backgroundPattern;

    // Animations
    QMap<CardItem*, QPropertyAnimation*> m_flipAnimations;
    QMap<CardItem*, QPropertyAnimation*> m_moveAnimations;
};

//==========================================================
//==========================================================

#endif // GAMESCENE_H
