#include "QMainWindow.h"
#include "ui_microwave.h"
#include "QtStateMachine/qstatemachine.h"
#include <QDir>

MaClasse::MaClasse(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MaClasse)
{
    ui->setupUi(this);
    stateMachine = new QStateMachine(this);

    QState* baseState = new QState(); //Base state for hierarchy setting
    stateMachine->addState(baseState);

    QState* idle = new QState(baseState);
    QState* setHours = new QState(baseState);
    QState* setMinutes = new QState(baseState);
    QState* setPower = new QState(baseState);
    QState* setCookingTime = new QState(baseState);
    QState* cooking = new QState(baseState);
    QState* setMode = new QState(baseState);
    QState* defrost = new QState(baseState);


    //We get the buttons from the ui
    startButton = ui->startButton;
    stopButton = ui->stopButton;
    powerButton = ui->powerButton;
    modeButton = ui->modeButton;
    defrostButton = ui->defrostButton;
    clockButton = ui->clockButton;
    stateLabel = ui->statusLabel;
    dial = ui->dial;
    time = ui->time;
    time->display(QString::number(0) + ":" + QString::number(0)); //default display

    //We set the transitions of the stateMachine
    idle->addTransition(startButton, &QPushButton::clicked, cooking);

    idle->addTransition(clockButton, &QPushButton::clicked, setHours);
    setHours->addTransition(clockButton, &QPushButton::clicked, setMinutes);
    setMinutes->addTransition(clockButton, &QPushButton::clicked, idle);

    idle->addTransition(powerButton, &QPushButton::clicked, setPower);
    setPower->addTransition(powerButton, &QPushButton::clicked, setCookingTime);
    setCookingTime->addTransition(startButton, &QPushButton::clicked, cooking);

    idle->addTransition(modeButton, &QPushButton::clicked, setMode);
    setMode->addTransition(modeButton, &QPushButton::clicked, setCookingTime);

    idle->addTransition(defrostButton, &QPushButton::clicked, defrost);
    defrost->addTransition(startButton, &QPushButton::clicked, cooking);

    cooking->addTransition(this, &MaClasse::finishedCooking, idle);

    baseState->addTransition(stopButton, &QPushButton::clicked, baseState); //All children of baseState will go back to idle when stopButton is clicked

    baseState->setInitialState(idle);

    //QTimer
    secondsTimer = new QTimer(this);
    secondsTimer->start(1000);
    cookingTimer = new QTimer(this);

    //OpenGLWidget placeholder replaced by our custom widget
    background = new MicrowaveOpenGLWidget(this);
    int index = ui->horiLayout->indexOf(ui->openGLWidget);
    if(index != -1){
        QLayoutItem* item = ui->horiLayout->takeAt(index);
        if(item->widget()){
            item->widget()->deleteLater();
            delete item;
        }
        ui->horiLayout->insertWidget(index, background);
        ui->horiLayout->setStretch(0, 1);
        ui->horiLayout->setStretch(1, 0);

    } else {
        ui->horiLayout->addWidget(background);
    }
    ui->horiLayout->update();

    //We connect the states to slots
    QObject::connect(idle, SIGNAL(entered()), this, SLOT(goIdle()));
    QObject::connect(idle, SIGNAL(exited()), this, SLOT(leavingIdle()));
    QObject::connect(setHours, SIGNAL(entered()), this, SLOT(setHours()));
    QObject::connect(setHours, SIGNAL(exited()), this, SLOT(finishSetHours()));
    QObject::connect(setMinutes, SIGNAL(entered()), this, SLOT(setMinutes()));
    QObject::connect(setMinutes, SIGNAL(exited()), this, SLOT(finishSetMinutes()));
    QObject::connect(setPower, SIGNAL(entered()), this, SLOT(setPower()));
    QObject::connect(setPower, SIGNAL(exited()), this, SLOT(finishSetPower()));
    QObject::connect(setCookingTime, SIGNAL(entered()), this, SLOT(setCookingTime()));
    QObject::connect(setCookingTime, SIGNAL(exited()), this, SLOT(finishSetCookingTime()));
    QObject::connect(cooking, SIGNAL(entered()), this, SLOT(cooking()));
    QObject::connect(cooking, SIGNAL(exited()), this, SLOT(leavingCooking()));
    QObject::connect(setMode, SIGNAL(entered()), this, SLOT(setMode()));
    QObject::connect(setMode, SIGNAL(exited()), this, SLOT(finishSetMode()));
    QObject::connect(defrost, SIGNAL(entered()), this, SLOT(defrost()));
    QObject::connect(defrost, SIGNAL(exited()), this, SLOT(leavingDefrost()));
    QObject::connect(startButton, SIGNAL(clicked()), this, SLOT(add30sec()));

    QObject::connect(secondsTimer, SIGNAL(timeout()), this, SLOT(changeSecond()));
    QObject::connect(cookingTimer, SIGNAL(timeout()), this, SLOT(changeCookingTime()));

    //We connect the dial
    QObject::connect(dial, SIGNAL(valueChanged(int)), this, SLOT(dialValueChanged()));

    stateMachine->setInitialState(baseState);
    stateMachine->start();
}

/////////////////////////////////////////////////SLOTS DEFINITIONS////////////////////////////////////////////////////////////////////
void MaClasse::changeSecond(){
    if(seconds < 59) seconds++;
    else{
        seconds = 0;
        if(minutes < 59)
            minutes ++;
        else{
            minutes = 0;
            if(hours < 23)
                hours ++;
            else{
                hours = 0;
            }
        }
    }
    secondsTimer->start(1000);
    if(settingHours || settingMinutes || beingIdle)
        time->display(QString("%1:%2").arg(hours < 10 ? QString("0%1").arg(hours) : QString::number(hours), minutes < 10 ? QString("0%1").arg(minutes) : QString::number(minutes)));

}

void MaClasse::changeCookingTime(){
    bool isFinished = false;
    if(cookingTimeSeconds > 1)
        cookingTimeSeconds--;
    else{
        if(cookingTimeMinutes == 0){
            cookingTimeSeconds = 0;
            isFinished = true;
        } else {
            cookingTimeMinutes--;
            cookingTimeSeconds = 59;
        }
    }
    if(!isFinished)
        cookingTimer->start(1000);
    else{
        cookingTimer->stop();
        //We force come back to Idle when the timer is finished COMPLETE HERE
        emit finishedCooking();
    }
    time->display(QString("%1:%2").arg(QString::number(cookingTimeMinutes), cookingTimeSeconds < 10 ? QString("0%1").arg(cookingTimeSeconds) : QString::number(cookingTimeSeconds)));
}

void MaClasse::add30sec(){
    if(isCooking){
        if(cookingTimeSeconds < 30)
            cookingTimeSeconds+= 30;
        else{
            cookingTimeMinutes++;
            cookingTimeSeconds = cookingTimeSeconds + 30 - 60;
        }
        time->display(QString("%1:%2").arg(QString::number(cookingTimeMinutes), cookingTimeSeconds < 10 ? QString("0%1").arg(cookingTimeSeconds) : QString::number(cookingTimeSeconds)));
    }
}

void MaClasse::dialValueChanged(){
    if(settingHours){
        hours = dial->value();
        time->display(QString("%1:%2").arg(hours < 10 ? QString("0%1").arg(hours) : QString::number(hours), minutes < 10 ? QString("0%1").arg(minutes) : QString::number(minutes)));

    }
    else if(settingMinutes){
        minutes = dial->value();
        time->display(QString("%1:%2").arg(hours < 10 ? QString("0%1").arg(hours) : QString::number(hours), minutes < 10 ? QString("0%1").arg(minutes) : QString::number(minutes)));

    }
    else if(settingPower){
        power = 10 * dial->value();
        time->display(QString::number(power));
    }
    else if(settingCookingTime){
        cookingTimeSeconds = dial->value();
        time->display(QString("0:%2").arg(cookingTimeSeconds < 10 ? QString("0%1").arg(cookingTimeSeconds) : QString::number(cookingTimeSeconds)));
    }
    else if(settingMode){
        mode = dial->value();
        switch (mode) {
        case 0:
            time->display(QString::number(0));
            break;
        case 1:
            time->display(QString::number(1));
            break;
        case 2:
            time->display(QString::number(2));
            break;
        case 3:
            time->display(QString::number(3));
            break;
        default:
            time->display(QString::number(0));
            break;
        }
    }
}

void MaClasse::goIdle()
{
    beingIdle = true;
    stateLabel->setText("Idle");
    time->display(QString("%1:%2").arg(hours < 10 ? QString("0%1").arg(hours) : QString::number(hours), minutes < 10 ? QString("0%1").arg(minutes) : QString::number(minutes)));

}

void MaClasse::leavingIdle(){
    beingIdle = false;
    justExitedIdle = true;
    justExitedNormalCooking = false;
}

void MaClasse::setHours()
{
    settingHours = true;
    dial->setRange(0,23);
    stateLabel->setText("Set Hours");
}

void MaClasse::setMinutes()
{
    settingMinutes = true;
    dial->setRange(0,59);
    stateLabel->setText("Set Minutes");
}

void MaClasse::finishSetHours()
{
    settingHours = false;
}

void MaClasse::finishSetMinutes()
{
    settingMinutes = false;
}

void MaClasse::setPower()
{
    settingPower = true;
    dial->setRange(0,20);
    stateLabel->setText("Set Power");
    time->display(QString::number(power));
}

void MaClasse::finishSetPower()
{
    settingPower = false;
}

void MaClasse::setCookingTime()
{
    settingCookingTime = true;
    dial->setRange(0,59);
    stateLabel->setText("Set Cooking Time");
    time->display(QString("0:%2").arg(cookingTimeSeconds < 10 ? QString("0%1").arg(cookingTimeSeconds) : QString::number(cookingTimeSeconds)));
}

void MaClasse::finishSetCookingTime()
{
    settingCookingTime = false;
    justExitedNormalCooking = true;
    justExitedIdle = false;

}

void MaClasse::cooking()
{
    stateLabel->setText("Cooking");
    if(justExitedIdle){
        cookingTimeMinutes = 0;
        cookingTimeSeconds = 30;
        justExitedIdle = false;
    } else if(justExitedNormalCooking){
        cookingTimeMinutes = 0;
    }
    time->display(QString("%1:%2").arg(QString::number(cookingTimeMinutes), cookingTimeSeconds < 10 ? QString("0%1").arg(cookingTimeSeconds) : QString::number(cookingTimeSeconds)));
    cookingTimer->start(1000);
    isCooking = true;
    background->startRotation();
    background->setColor(Qt::red);
}

void MaClasse::leavingCooking(){
    cookingTimer->stop();
    isCooking = false;
    background->stopRotation();
    background->setColor(Qt::white);
}

void MaClasse::setMode()
{
    settingMode = true;
    dial->setRange(0, 3);
    stateLabel->setText("Set Mode");
    switch (mode) {
    case 0:
        time->display(QString::number(0));
        break;
    case 1:
        time->display(QString::number(1));
        break;
    case 2:
        time->display(QString::number(2));
        break;
    case 3:
        time->display(QString::number(3));
        break;
    default:
        time->display(QString::number(0));
        break;
    }
}

void MaClasse::finishSetMode()
{
    settingMode = false;
}

void MaClasse::defrost()
{
    stateLabel->setText("Defrost");
}

void MaClasse::leavingDefrost(){
    justExitedIdle = false;
    justExitedNormalCooking = true;
    cookingTimeMinutes = 0;
    //We assign to cooking time seconds a random number between 15 and 59
    cookingTimeSeconds = rand() % 45 + 15;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MaClasse::~MaClasse()
{
    delete ui;
}

