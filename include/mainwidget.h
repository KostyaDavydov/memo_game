#ifndef MAINWIDGET_H
#define MAINWIDGET_H

//==========================================================
//==========================================================

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QStatusBar>
#include <QProgressBar>

//==========================================================
//==========================================================

class CardItem;
class GameScene;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private slots:
    void onStartGame();
    void onResetGame();
    void onCardClicked(int cardId);
    void onDifficultyChanged(int index);
    void onCardCountChanged(int value);
    void updateGameInfo();

private:
    void setupUI();
    void createCards();
    void setupConnections();
    void updateScore(int newScore);

    // UI Components
    QGraphicsView *m_gameView;
    GameScene *m_gameScene;

    // Control Panel
    QGroupBox *m_controlGroup;
    QPushButton *m_startButton;
    QPushButton *m_resetButton;
    QPushButton *m_hintButton;
    QPushButton *m_undoButton;

    // Settings
    QGroupBox *m_settingsGroup;
    QComboBox *m_difficultyCombo;
    QSpinBox *m_cardCountSpin;
    QComboBox *m_themeCombo;

    // Game Info
    QGroupBox *m_infoGroup;
    QLabel *m_scoreLabel;
    QLabel *m_movesLabel;
    QLabel *m_timeLabel;
    QLabel *m_matchesLabel;
    QProgressBar *m_progressBar;

    // Status Bar
    QStatusBar *m_statusBar;

    // Layouts
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_gameLayout;
    QVBoxLayout *m_sidePanelLayout;

    // Game State
    int m_score;
    int m_moves;
    int m_matches;
    QTimer *m_gameTimer;
};

//==========================================================
//==========================================================

#endif // MAINWIDGET_H
