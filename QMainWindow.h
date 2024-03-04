#ifndef MACLASSE_H
#define MACLASSE_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QDial>
#include <QStateMachine>
#include <QState>
#include <QLCDNumber>
#include <QTimer>
#include <QOpenGLWidget>
#include <QColor>
#include "MicrowaveOpenGLWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MaClasse; }
QT_END_NAMESPACE

class MaClasse : public QMainWindow
{
    Q_OBJECT

public:
    MaClasse(QWidget *parent = nullptr);
    ~MaClasse();

signals:
    void finishedCooking();

private slots:
    void setHours();
    void setMinutes();
    void setPower();
    void setCookingTime();
    void setMode();
    void defrost();
    void cooking();
    void goIdle();

    void finishSetHours();
    void finishSetMinutes();
    void finishSetPower();
    void finishSetMode();
    void finishSetCookingTime();
    void leavingCooking();
    void leavingDefrost();
    void leavingIdle();

    void dialValueChanged();

    void changeSecond();
    void changeCookingTime();
    void add30sec();

private:
    QStateMachine* stateMachine;

    Ui::MaClasse *ui;
    QDial* dial;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *powerButton;
    QPushButton *modeButton;
    QPushButton *defrostButton;
    QPushButton *clockButton;
    QLabel *stateLabel;
    QLCDNumber* time;

    QTimer* secondsTimer;
    QTimer* cookingTimer;
    MicrowaveOpenGLWidget* background;

    bool settingHours = false;
    bool settingMinutes = false;
    bool settingPower = false;
    bool settingMode = false;
    bool settingCookingTime = false;
    bool beingIdle = true;
    bool isCooking = false;

    bool justExitedIdle = false;
    bool justExitedNormalCooking = false;

    int hours = 0;
    int minutes = 0;
    int power = 0;
    int cookingTimeSeconds = 0;
    int cookingTimeMinutes = 0;
    int mode = 0;

    int seconds = 0;
};
#endif // MACLASSE_H
