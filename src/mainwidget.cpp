#include "mainwidget.h"
#include "gamescene.h"
#include "carditem.h"

#include <QTimer>
#include <QTime>
#include <QMessageBox>
#include <QFormLayout>

//==========================================================
//==========================================================

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , m_score(0)
    , m_moves(0)
    , m_matches(0)
{
    setupUI();
    setupConnections();

    // Initialize with default values
    updateGameInfo();

    // Set window properties
    setWindowTitle("Memory Card Game");
    setMinimumSize(1000, 700);
    resize(1200, 800);
}

//==========================================================

MainWidget::~MainWidget()
{
    delete m_gameTimer;
}

//==========================================================

void MainWidget::setupUI()
{
    // Create main layout
    m_mainLayout = new QVBoxLayout(this);

    // Create game layout (view + side panel)
    m_gameLayout = new QHBoxLayout();

    // Create graphics scene and view
    m_gameScene = new GameScene(this);
    m_gameView = new QGraphicsView(m_gameScene);
    m_gameView->setRenderHint(QPainter::Antialiasing);
    m_gameView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    m_gameView->setBackgroundBrush(QBrush(QColor("#2c3e50")));
    m_gameView->setCacheMode(QGraphicsView::CacheBackground);

    // Create side panel
    QWidget *sidePanel = new QWidget();
    sidePanel->setMaximumWidth(300);
    m_sidePanelLayout = new QVBoxLayout(sidePanel);
    m_sidePanelLayout->setSpacing(15);

    // ===== Control Group =====
    m_controlGroup = new QGroupBox("Game Controls");
    QVBoxLayout *controlLayout = new QVBoxLayout();

    m_startButton = new QPushButton("Start Game");
    m_startButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; font-weight: bold; padding: 10px; }");

    m_resetButton = new QPushButton("Reset");
    m_resetButton->setEnabled(false);

    m_hintButton = new QPushButton("Hint");
    m_hintButton->setEnabled(false);

    m_undoButton = new QPushButton("Undo");
    m_undoButton->setEnabled(false);

    controlLayout->addWidget(m_startButton);
    controlLayout->addWidget(m_resetButton);
    controlLayout->addWidget(m_hintButton);
    controlLayout->addWidget(m_undoButton);
    m_controlGroup->setLayout(controlLayout);

    // ===== Settings Group =====
    m_settingsGroup = new QGroupBox("Game Settings");
    QFormLayout *settingsLayout = new QFormLayout();

    m_difficultyCombo = new QComboBox();
    m_difficultyCombo->addItems({"Easy", "Medium", "Hard"});

    m_cardCountSpin = new QSpinBox();
    m_cardCountSpin->setRange(8, 40);
    m_cardCountSpin->setValue(16);
    m_cardCountSpin->setSingleStep(4);

    m_themeCombo = new QComboBox();
    m_themeCombo->addItems({"Animals", "Numbers", "Symbols", "Flags", "Custom"});

    settingsLayout->addRow("Difficulty:", m_difficultyCombo);
    settingsLayout->addRow("Card Count:", m_cardCountSpin);
    settingsLayout->addRow("Theme:", m_themeCombo);
    m_settingsGroup->setLayout(settingsLayout);

    // ===== Game Info Group =====
    m_infoGroup = new QGroupBox("Game Information");
    QFormLayout *infoLayout = new QFormLayout();

    m_scoreLabel = new QLabel("0");
    m_scoreLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 16px; color: #e74c3c; }");

    m_movesLabel = new QLabel("0");
    m_timeLabel = new QLabel("00:00");
    m_matchesLabel = new QLabel("0/0");

    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat("%p% Complete");

    infoLayout->addRow("Score:", m_scoreLabel);
    infoLayout->addRow("Moves:", m_movesLabel);
    infoLayout->addRow("Time:", m_timeLabel);
    infoLayout->addRow("Matches:", m_matchesLabel);
    infoLayout->addRow(m_progressBar);
    m_infoGroup->setLayout(infoLayout);

    // Add groups to side panel
    m_sidePanelLayout->addWidget(m_controlGroup);
    m_sidePanelLayout->addWidget(m_settingsGroup);
    m_sidePanelLayout->addWidget(m_infoGroup);
    m_sidePanelLayout->addStretch();

    // Add view and side panel to game layout
    m_gameLayout->addWidget(m_gameView, 4);
    m_gameLayout->addWidget(sidePanel, 1);

    // ===== Status Bar =====
    m_statusBar = new QStatusBar();
    m_statusBar->setSizeGripEnabled(false);
    m_statusBar->showMessage("Ready to start a new game!");

    // Add everything to main layout
    m_mainLayout->addLayout(m_gameLayout, 4);
    m_mainLayout->addWidget(m_statusBar, 1);

    // Create game timer
    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(1000); // 1 second
}

//==========================================================

void MainWidget::setupConnections()
{
    // Button connections
    connect(m_startButton, &QPushButton::clicked, this, &MainWidget::onStartGame);
    connect(m_resetButton, &QPushButton::clicked, this, &MainWidget::onResetGame);

    // Settings connections
    connect(m_difficultyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWidget::onDifficultyChanged);
    connect(m_cardCountSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWidget::onCardCountChanged);

    // Timer connection
    connect(m_gameTimer, &QTimer::timeout, this, &MainWidget::updateGameInfo);

    // Connect to card signals (assuming CardItem emits a signal when clicked)
    // This will be connected when cards are created
}

//==========================================================

void MainWidget::onStartGame()
{
    // Disable settings during game
    m_difficultyCombo->setEnabled(false);
    m_cardCountSpin->setEnabled(false);
    m_themeCombo->setEnabled(false);

    // Enable game controls
    m_resetButton->setEnabled(true);
    m_hintButton->setEnabled(true);
    m_undoButton->setEnabled(true);
    m_startButton->setEnabled(false);
    m_startButton->setText("Game Running");

    // Reset game state
    m_score = 0;
    m_moves = 0;
    m_matches = 0;

    // Clear existing cards and create new ones
    m_gameScene->clear();
    createCards();

    // Start game timer
    m_gameTimer->start();

    // Update status
    m_statusBar->showMessage("Game started! Find matching cards.");
    updateScore(0);
}

//==========================================================

void MainWidget::onResetGame()
{
    // Stop timer
    m_gameTimer->stop();

    // Re-enable settings
    m_difficultyCombo->setEnabled(true);
    m_cardCountSpin->setEnabled(true);
    m_themeCombo->setEnabled(true);

    // Disable game controls
    m_resetButton->setEnabled(false);
    m_hintButton->setEnabled(false);
    m_undoButton->setEnabled(false);
    m_startButton->setEnabled(true);
    m_startButton->setText("Start Game");

    // Clear scene
    m_gameScene->clear();

    // Reset labels
    m_timeLabel->setText("00:00");
    m_matchesLabel->setText("0/0");
    m_progressBar->setValue(0);

    // Update status
    m_statusBar->showMessage("Game reset. Ready to start a new game!");
}

//==========================================================

void MainWidget::onCardClicked(int cardId)
{
    // Increment moves
    m_moves++;

    // Update moves label
    m_movesLabel->setText(QString::number(m_moves));

    // Update status
    m_statusBar->showMessage(QString("Card %1 clicked. Moves: %2").arg(cardId).arg(m_moves));

    // Check for game completion
    int totalPairs = m_cardCountSpin->value() / 2;
    if (m_matches >= totalPairs) {
        m_gameTimer->stop();
        QMessageBox::information(this, "Congratulations!",
                                 QString("You won!\nScore: %1\nMoves: %2").arg(m_score).arg(m_moves));
    }
}

//==========================================================

void MainWidget::onDifficultyChanged(int index)
{
    QString difficulty = m_difficultyCombo->itemText(index);
    m_statusBar->showMessage(QString("Difficulty set to: %1").arg(difficulty));

    // Adjust card count based on difficulty
    switch(index) {
    case 0: // Easy
        m_cardCountSpin->setValue(12);
        break;
    case 1: // Medium
        m_cardCountSpin->setValue(16);
        break;
    case 2: // Hard
        m_cardCountSpin->setValue(24);
        break;
    }
}

//==========================================================

void MainWidget::onCardCountChanged(int value)
{
    // Ensure even number of cards
    if (value % 2 != 0) {
        m_cardCountSpin->setValue(value + 1);
        return;
    }

    m_statusBar->showMessage(QString("Card count set to: %1").arg(value));
    m_matchesLabel->setText(QString("0/%1").arg(value / 2));
}

//==========================================================

void MainWidget::updateGameInfo()
{
    // Update time (simplified - you'd track actual elapsed time)
    static int seconds = 0;
    seconds++;

    QTime time(0, 0, 0);
    time = time.addSecs(seconds);
    m_timeLabel->setText(time.toString("mm:ss"));

    // Update progress
    int totalPairs = m_cardCountSpin->value() / 2;
    if (totalPairs > 0) {
        int progress = (m_matches * 100) / totalPairs;
        m_progressBar->setValue(progress);
    }
}

//==========================================================

void MainWidget::updateScore(int newScore)
{
    m_score = newScore;
    m_scoreLabel->setText(QString::number(m_score));

    // Add some visual feedback for score changes
    if (newScore > 0) {
        m_scoreLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 16px; color: #27ae60; }");
        QTimer::singleShot(500, this, [this]() {
            m_scoreLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 16px; color: #e74c3c; }");
        });
    }
}

//==========================================================

void MainWidget::createCards()
{
    int cardCount = m_cardCountSpin->value();
    int cardWidth = 100;
    int cardHeight = 140;
    int spacing = 10;

    // Calculate grid dimensions
    int cols = (cardCount <= 16) ? 4 : 6;
    int rows = (cardCount + cols - 1) / cols;

    // Calculate starting position to center the grid
    int totalWidth = cols * (cardWidth + spacing) - spacing;
    int totalHeight = rows * (cardHeight + spacing) - spacing;

    int startX = (m_gameScene->width() - totalWidth) / 2;
    int startY = (m_gameScene->height() - totalHeight) / 2;

    // Create cards
    for (int i = 0; i < cardCount; i++) {
        int row = i / cols;
        int col = i % cols;

        int x = startX + col * (cardWidth + spacing);
        int y = startY + row * (cardHeight + spacing);

        // Create card item (you'll need to implement CardItem class)
        CardItem *card = new CardItem(i, 1, QRectF(0, 0, cardWidth, cardHeight));
        card->setPos(x, y);
        m_gameScene->addItem(card);

        // Connect card signals
        // Assuming CardItem has a clicked signal:
        // connect(card, &CardItem::clicked, this, &MainWidget::onCardClicked);
    }

    m_matchesLabel->setText(QString("0/%1").arg(cardCount / 2));
}

