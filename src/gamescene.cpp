#include "gamescene.h"
#include "carditem.h"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QTime>
#include <QRandomGenerator>
#include <QtMath>

//==========================================================
//==========================================================

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_gameActive(false)
    , m_gamePaused(false)
    , m_waitingForCheck(false)
    , m_score(0)
    , m_moves(0)
    , m_matches(0)
    , m_totalPairs(0)
    , m_hintsRemaining(3)
    , m_elapsedTime(0)
    , m_columns(4)
    , m_rows(4)
    , m_cardWidth(100)
    , m_cardHeight(140)
    , m_cardSpacing(15)
    , m_lastClickedCard(nullptr)
    , m_hintCard1(nullptr)
    , m_hintCard2(nullptr)
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

    // Setup timers
    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(1000); // 1 second

    m_matchCheckTimer = new QTimer(this);
    m_matchCheckTimer->setSingleShot(true);
    m_matchCheckTimer->setInterval(1000); // 1 second delay for match checking

    m_hintTimer = new QTimer(this);
    m_hintTimer->setSingleShot(true);
    m_hintTimer->setInterval(2000); // 2 second hint duration

    // Connect timer signals
    connect(m_gameTimer, &QTimer::timeout, this, &GameScene::updateTimer);
    connect(m_matchCheckTimer, &QTimer::timeout, this, &GameScene::onMatchCheckTimeout);
    connect(m_hintTimer, &QTimer::timeout, this, &GameScene::onHintTimeout);
}

//==========================================================

GameScene::~GameScene()
{
    clearScene();
}

//==========================================================

void GameScene::setupScene(int cardCount, int difficulty)
{
    clearScene();

    // Validate card count (must be even)
    if (cardCount % 2 != 0) {
        cardCount++;
    }

    m_totalPairs = cardCount / 2;

    // Set difficulty-based parameters
    switch (difficulty) {
    case 0: // Easy
        m_cardWidth = 120;
        m_cardHeight = 160;
        m_cardSpacing = 20;
        break;
    case 1: // Medium
        m_cardWidth = 100;
        m_cardHeight = 140;
        m_cardSpacing = 15;
        break;
    case 2: // Hard
        m_cardWidth = 80;
        m_cardHeight = 120;
        m_cardSpacing = 10;
        break;
    }

    // Calculate grid layout
    calculateGrid(cardCount);

    // Initialize cards
    initializeCards();

    // Layout cards
    layoutCards();

    // Update game state
    updateGameState("Setup Complete");

    emit gameStateChanged("Ready");
}

//==========================================================

void GameScene::clearScene()
{
    // Stop all animations
    for (auto animation : m_flipAnimations) {
        animation->stop();
    }
    for (auto animation : m_moveAnimations) {
        animation->stop();
    }
    m_flipAnimations.clear();
    m_moveAnimations.clear();

    // Clear timers
    m_gameTimer->stop();
    m_matchCheckTimer->stop();
    m_hintTimer->stop();

    // Clear card lists
    m_flippedCards.clear();
    m_matchedCards.clear();
    m_lastClickedCard = nullptr;
    m_hintCard1 = nullptr;
    m_hintCard2 = nullptr;

    // Remove all items from scene
    clear();

    // Clear card pointers
    qDeleteAll(m_cards);
    m_cards.clear();

    m_gameActive = false;
    m_gamePaused = false;
    m_waitingForCheck = false;
}

//==========================================================

void GameScene::resetCards()
{
    if (!m_gameActive) return;

    // Flip all cards face down
    flipAllCards(false, true);

    // Shuffle cards
    shuffleCards();

    // Reset tracking lists
    m_flippedCards.clear();
    m_matchedCards.clear();
    m_lastClickedCard = nullptr;

    // Reset moves and matches but keep score
    m_moves = 0;
    m_matches = 0;

    emit movesChanged(0);
    emit matchesChanged(0);

    updateGameState("Cards Reset");
}

//==========================================================

void GameScene::initializeCards()
{
    // Create pairs of cards
    QList<int> cardValues;
    for (int i = 0; i < m_totalPairs; i++) {
        cardValues.append(i);
        cardValues.append(i);
    }

    // Shuffle the values
    std::random_shuffle(cardValues.begin(), cardValues.end());

    // Create card items
    for (int i = 0; i < cardValues.size(); i++) {
        int cardValue = cardValues[i];
        CardItem *card = new CardItem(i, cardValue, QRectF(0, 0, m_cardWidth, m_cardHeight));

        // Set card appearance
        card->setCardBack(createCardBack());
        card->setCardFace(createCardFace(cardValue % 8, cardValue / 8));

        // Connect signals
        connect(card, &CardItem::clicked, this, [this, card]() {
            processCardClick(card);
        });

        // Add to scene and tracking lists
        addItem(card);
        m_cards.append(card);
    }
}

//==========================================================

void GameScene::layoutCards()
{
    QRectF sceneBounds = sceneRect();
    QPointF gridCenter(sceneBounds.center());

    // Calculate total grid size
    float gridWidth = m_columns * (m_cardWidth + m_cardSpacing) - m_cardSpacing;
    float gridHeight = m_rows * (m_cardHeight + m_cardSpacing) - m_cardSpacing;

    // Calculate starting position (top-left of grid)
    float startX = gridCenter.x() - gridWidth / 2;
    float startY = gridCenter.y() - gridHeight / 2;

    // Position each card
    for (int i = 0; i < m_cards.size(); i++) {
        int row = i / m_columns;
        int col = i % m_columns;

        float x = startX + col * (m_cardWidth + m_cardSpacing);
        float y = startY + row * (m_cardHeight + m_cardSpacing);

        m_cards[i]->setPos(x, y);
        m_cards[i]->setZValue(0);
    }
}

//==========================================================

void GameScene::calculateGrid(int cardCount)
{
    // Try to create a roughly square grid
    int sqrtVal = qSqrt(cardCount);
    m_columns = sqrtVal;
    m_rows = (cardCount + m_columns - 1) / m_columns;

    // Adjust for better aspect ratio
    while (m_columns * m_rows > cardCount) {
        if (m_columns > m_rows) {
            m_columns--;
        } else {
            m_rows--;
        }
    }
}

//==========================================================

QRectF GameScene::getGridBounds() const
{
    if (m_cards.isEmpty()) {
        return QRectF();
    }

    QRectF bounds;
    for (CardItem* card : m_cards) {
        bounds = bounds.united(card->sceneBoundingRect());
    }

    return bounds;
}

//==========================================================

void GameScene::startGame()
{
    if (m_gameActive) return;

    m_gameActive = true;
    m_gamePaused = false;
    m_score = 0;
    m_moves = 0;
    m_matches = 0;
    m_elapsedTime = 0;
    m_hintsRemaining = 3;

    // Shuffle cards
    shuffleCards();

    // Flip all cards face down
    flipAllCards(false, false);

    // Start game timer
    m_gameTimer->start();

    updateGameState("Game Started");

    emit gameStarted();
    emit scoreChanged(0);
    emit movesChanged(0);
    emit matchesChanged(0);
}

//==========================================================

void GameScene::pauseGame(bool paused)
{
    if (!m_gameActive) return;

    m_gamePaused = paused;

    if (paused) {
        m_gameTimer->stop();
        updateGameState("Game Paused");
    } else {
        m_gameTimer->start();
        updateGameState("Game Resumed");
    }

    emit gamePaused(paused);
}

//==========================================================

void GameScene::endGame()
{
    if (!m_gameActive) return;

    m_gameActive = false;
    m_gameTimer->stop();

    // Flip all cards face up to show solution
    flipAllCards(true, true);

    updateGameState("Game Ended");

    emit gameEnded(m_score, m_moves, m_elapsedTime);
}

//==========================================================

void GameScene::shuffleCards()
{
    if (m_cards.isEmpty()) return;

    // Get current positions
    QList<QPointF> positions;
    for (CardItem* card : m_cards) {
        positions.append(card->pos());
    }

    // Shuffle positions
    std::random_shuffle(positions.begin(), positions.end());

    // Animate cards to new positions
    for (int i = 0; i < m_cards.size(); i++) {
        if (m_moveAnimations.contains(m_cards[i])) {
            m_moveAnimations[m_cards[i]]->stop();
        }

        QPropertyAnimation *animation = new QPropertyAnimation(m_cards[i], "pos");
        animation->setDuration(500);
        animation->setStartValue(m_cards[i]->pos());
        animation->setEndValue(positions[i]);
        animation->setEasingCurve(QEasingCurve::InOutQuad);

        connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);

        m_moveAnimations[m_cards[i]] = animation;
        animation->start();
    }

    updateGameState("Cards Shuffled");
}

//==========================================================

void GameScene::processCardClick(CardItem* card)
{
    if (!m_gameActive || m_gamePaused || m_waitingForCheck) return;

    // Ignore if card is already matched or flipped
    if (card->isMatched() || card->isFlipped()) return;

    // Flip the card
    flipCard(card, true);

    // Add to flipped cards list
    m_flippedCards.append(card);
    m_lastClickedCard = card;

    // Increment moves
    incrementMoves();

    // Check if we have two cards flipped
    if (m_flippedCards.size() == 2) {
        m_waitingForCheck = true;
        m_matchCheckTimer->start(); // Wait before checking match
    }

    emit cardFlipped(card);
}

//==========================================================

void GameScene::checkForMatch()
{
    if (m_flippedCards.size() != 2) return;

    CardItem* card1 = m_flippedCards[0];
    CardItem* card2 = m_flippedCards[1];

    bool isMatch = (card1->getCardValue() == card2->getCardValue());

    if (isMatch) {
        // Match found
        card1->setMatched(true);
        card2->setMatched(true);

        m_matchedCards.append(card1);
        m_matchedCards.append(card2);

        incrementMatches();
        addScore(100); // Base score for match

        // Bonus for quick matches
        if (m_moves < m_totalPairs * 2) {
            addScore(50);
        }

        // Animation for matched cards
        highlightCard(card1, true);
        highlightCard(card2, true);

        emit matchFound(card1, card2);

        // Check if game is complete
        if (m_matches >= m_totalPairs) {
            endGame();
        }
    } else {
        // No match - flip cards back after delay
        emit noMatchFound(card1, card2);

        // Penalty for wrong match
        addScore(-10);
    }

    // Clear flipped cards list after processing
    m_flippedCards.clear();
    m_waitingForCheck = false;
}

//==========================================================

void GameScene::onFlipAnimationFinished()
{
    // Handle any cleanup after flip animations
}

//==========================================================

void GameScene::onMatchCheckTimeout()
{
    checkForMatch();
}

//==========================================================

void GameScene::onHintTimeout()
{
    // Remove hint highlighting
    if (m_hintCard1) {
        highlightCard(m_hintCard1, false);
        m_hintCard1 = nullptr;
    }
    if (m_hintCard2) {
        highlightCard(m_hintCard2, false);
        m_hintCard2 = nullptr;
    }
}

//==========================================================

void GameScene::updateTimer()
{
    m_elapsedTime++;

    // Update game state every 30 seconds
    if (m_elapsedTime % 30 == 0) {
        updateGameState(QString("Playing - %1 seconds elapsed").arg(m_elapsedTime));
    }
}

//==========================================================

void GameScene::flipCard(CardItem* card, bool faceUp, bool animated)
{
    if (animated) {
        // Create flip animation
        QPropertyAnimation *animation = new QPropertyAnimation(card, "rotation");
        animation->setDuration(300);
        animation->setStartValue(faceUp ? 0 : 180);
        animation->setEndValue(faceUp ? 180 : 0);
        animation->setEasingCurve(QEasingCurve::InOutQuad);

        connect(animation, &QPropertyAnimation::finished, [this, card, faceUp, animation]() {
            card->setFlipped(faceUp);
            m_flipAnimations.remove(card);
            animation->deleteLater();
            onFlipAnimationFinished();
        });

        m_flipAnimations[card] = animation;
        animation->start();
    } else {
        card->setFlipped(faceUp);
    }
}

//==========================================================

void GameScene::flipAllCards(bool faceUp, bool animated)
{
    for (CardItem* card : m_cards) {
        if (!card->isMatched()) {
            flipCard(card, faceUp, animated);
        }
    }
}

//==========================================================

void GameScene::highlightCard(CardItem* card, bool highlight)
{
    if (!card) return;

    if (highlight) {
        card->setGlowEffect(true, QColor("#f1c40f"));
        card->setZValue(10);
    } else {
        card->setGlowEffect(false);
        card->setZValue(0);
    }
}

//==========================================================

void GameScene::showHint()
{
    if (!m_gameActive || m_gamePaused || m_hintsRemaining <= 0) return;

    m_hintsRemaining--;

    // Find a pair of unmatched cards
    QList<CardItem*> unmatchedCards;
    for (CardItem* card : m_cards) {
        if (!card->isMatched() && !card->isFlipped()) {
            unmatchedCards.append(card);
        }
    }

    if (unmatchedCards.size() >= 2) {
        // Find a matching pair
        QMap<int, CardItem*> valueMap;
        for (CardItem* card : unmatchedCards) {
            int value = card->getCardValue();
            if (valueMap.contains(value)) {
                m_hintCard1 = valueMap[value];
                m_hintCard2 = card;
                break;
            }
            valueMap[value] = card;
        }

        // If no match found, just pick two random cards
        if (!m_hintCard1 || !m_hintCard2) {
            m_hintCard1 = unmatchedCards[QRandomGenerator::global()->bounded(unmatchedCards.size())];
            do {
                m_hintCard2 = unmatchedCards[QRandomGenerator::global()->bounded(unmatchedCards.size())];
            } while (m_hintCard1 == m_hintCard2);
        }

        // Highlight the hint cards
        highlightCard(m_hintCard1, true);
        highlightCard(m_hintCard2, true);

        // Start timer to remove highlight
        m_hintTimer->start();

        updateGameState(QString("Hint Used - %1 remaining").arg(m_hintsRemaining));
    }

    emit hintRequested();
}

//==========================================================

QPixmap GameScene::createCardBack(int type) const
{
    QPixmap pixmap(m_cardWidth, m_cardHeight);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw card background with rounded corners
    QPainterPath path;
    path.addRoundedRect(1, 1, m_cardWidth - 2, m_cardHeight - 2, 10, 10);

    // Gradient background
    QLinearGradient gradient(0, 0, m_cardWidth, m_cardHeight);
    gradient.setColorAt(0, QColor("#3498db"));
    gradient.setColorAt(1, QColor("#2980b9"));

    painter.fillPath(path, gradient);
    painter.setPen(QPen(QColor("#2c3e50"), 2));
    painter.drawPath(path);

    // Draw pattern based on type
    painter.setPen(QPen(QColor("#ecf0f1"), 2));
    switch (type) {
    case 0: // Default pattern
        painter.drawEllipse(m_cardWidth / 2 - 15, m_cardHeight / 2 - 15, 30, 30);
        break;
    case 1: // Diamond pattern
        painter.drawLine(m_cardWidth / 2, 20, m_cardWidth - 20, m_cardHeight / 2);
        painter.drawLine(m_cardWidth / 2, 20, 20, m_cardHeight / 2);
        painter.drawLine(20, m_cardHeight / 2, m_cardWidth / 2, m_cardHeight - 20);
        painter.drawLine(m_cardWidth - 20, m_cardHeight / 2, m_cardWidth / 2, m_cardHeight - 20);
        break;
    }

    return pixmap;
}

//==========================================================

QPixmap GameScene::createCardFace(int value, int suit) const
{
    QPixmap pixmap(m_cardWidth, m_cardHeight);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw card background
    QPainterPath path;
    path.addRoundedRect(1, 1, m_cardWidth - 2, m_cardHeight - 2, 10, 10);

    painter.fillPath(path, Qt::white);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawPath(path);

    // Set colors based on suit
    QColor symbolColor;
    QString symbol;

    switch (suit % 4) {
    case 0: // Hearts
        symbolColor = QColor("#e74c3c");
        symbol = "♥";
        break;
    case 1: // Diamonds
        symbolColor = QColor("#e74c3c");
        symbol = "♦";
        break;
    case 2: // Clubs
        symbolColor = Qt::black;
        symbol = "♣";
        break;
    case 3: // Spades
        symbolColor = Qt::black;
        symbol = "♠";
        break;
    }

    painter.setPen(symbolColor);

    // Draw value and symbol
    QFont font = painter.font();
    font.setPointSize(14);
    font.setBold(true);
    painter.setFont(font);

    QString valueStr;
    switch (value % 13) {
    case 0: valueStr = "A"; break;
    case 10: valueStr = "J"; break;
    case 11: valueStr = "Q"; break;
    case 12: valueStr = "K"; break;
    default: valueStr = QString::number((value % 13) + 1); break;
    }

    // Draw top-left value and symbol
    painter.drawText(10, 25, valueStr);
    painter.drawText(10, 45, symbol);

    // Draw bottom-right value and symbol (rotated)
    painter.save();
    painter.translate(m_cardWidth - 20, m_cardHeight - 20);
    painter.rotate(180);
    painter.drawText(0, 0, valueStr);
    painter.drawText(0, 20, symbol);
    painter.restore();

    // Draw large center symbol
    font.setPointSize(36);
    painter.setFont(font);
    QRectF centerRect(m_cardWidth / 2 - 25, m_cardHeight / 2 - 25, 50, 50);
    painter.drawText(centerRect, Qt::AlignCenter, symbol);

    return pixmap;
}

//==========================================================

void GameScene::updateGameState(const QString &state)
{
    qDebug() << "Game State:" << state;
    emit gameStateChanged(state);
}

//==========================================================

void GameScene::addScore(int points)
{
    m_score += points;
    if (m_score < 0) m_score = 0;

    emit scoreChanged(m_score);
}

//==========================================================

void GameScene::incrementMoves()
{
    m_moves++;
    emit movesChanged(m_moves);
}

//==========================================================

void GameScene::incrementMatches()
{
    m_matches++;
    emit matchesChanged(m_matches);
}

//==========================================================

// Event Handlers
void GameScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);

    if (event->button() == Qt::RightButton && m_gameActive) {
        // Right click for quick hint
        showHint();
    }
}

//==========================================================

void GameScene::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Space:
        if (m_gameActive) {
            pauseGame(!m_gamePaused);
        }
        break;
    case Qt::Key_H:
        if (m_gameActive && !m_gamePaused) {
            showHint();
        }
        break;
    case Qt::Key_R:
        if (m_gameActive) {
            resetCards();
        }
        break;
    case Qt::Key_Escape:
        if (m_gameActive) {
            endGame();
        }
        break;
    }

    QGraphicsScene::keyPressEvent(event);
}

//==========================================================

void GameScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    // Fill with base color
    painter->fillRect(rect, m_backgroundBrush);

    // Draw tiled pattern
    painter->setOpacity(0.1);
    for (int x = rect.left(); x < rect.right(); x += 64) {
        for (int y = rect.top(); y < rect.bottom(); y += 64) {
            painter->drawPixmap(x, y, m_backgroundPattern);
        }
    }
    painter->setOpacity(1.0);

    // Draw grid lines if game is active
    if (m_gameActive && !m_cards.isEmpty()) {
        QRectF gridBounds = getGridBounds();
        if (!gridBounds.isEmpty()) {
            painter->setPen(QPen(m_gridColor, 2));
            painter->drawRect(gridBounds.adjusted(-10, -10, 10, 10));
        }
    }
}

//==========================================================//==========================================================

void GameScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawForeground(painter, rect);

    // Draw pause overlay if game is paused
    if (m_gamePaused) {
        painter->setBrush(QColor(0, 0, 0, 180));
        painter->setPen(Qt::NoPen);
        painter->drawRect(rect);

        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 48, QFont::Bold));
        painter->drawText(rect, Qt::AlignCenter, "PAUSED");

        painter->setFont(QFont("Arial", 24));
        painter->drawText(rect.adjusted(0, 100, 0, 0), Qt::AlignCenter, "Press SPACE to resume");
    }

    // Draw game over overlay if game ended
    if (!m_gameActive && m_elapsedTime > 0) {
        painter->setBrush(QColor(0, 0, 0, 200));
        painter->setPen(Qt::NoPen);
        painter->drawRect(rect);

        painter->setPen(QColor("#f1c40f"));
        painter->setFont(QFont("Arial", 36, QFont::Bold));
        painter->drawText(rect.adjusted(0, -100, 0, 0), Qt::AlignCenter, "GAME COMPLETE!");

        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 24));

        QString stats = QString("Score: %1\nMoves: %2\nTime: %3 seconds")
                            .arg(m_score)
                            .arg(m_moves)
                            .arg(m_elapsedTime);

        painter->drawText(rect, Qt::AlignCenter, stats);
    }
}

//==========================================================

QList<CardItem*> GameScene::getCards() const
{
    return m_cards;
}

//==========================================================

CardItem* GameScene::getCardAt(const QPointF &pos) const
{
    QList<QGraphicsItem*> itemsAtPos = items(pos);
    for (QGraphicsItem* item : itemsAtPos) {
        if (CardItem* card = dynamic_cast<CardItem*>(item)) {
            return card;
        }
    }
    return nullptr;
}

//==========================================================

QList<CardItem*> GameScene::getSelectedCards() const
{
    return m_flippedCards;
}

//==========================================================

void GameScene::setGridSize(int columns, int rows)
{
    m_columns = columns;
    m_rows = rows;
    layoutCards();
}

