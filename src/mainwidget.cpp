#include "mainwidget.h"
#include "gamescene.h"

//==========================================================
//==========================================================

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();

    // Set window properties
    setWindowTitle("Memory Card Game");
    setMinimumSize(1000, 700);
    resize(1200, 800);
}

//==========================================================

MainWidget::~MainWidget()
{
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
}
