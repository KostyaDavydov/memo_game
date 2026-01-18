#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QGraphicsView>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QProgressBar>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>

//==========================================================
//==========================================================

class GameScene;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private:
    void setupUI();

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
};

//==========================================================
//==========================================================

#endif // MAINWIDGET_H
