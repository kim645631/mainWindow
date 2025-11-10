#include "mainwindow.h"
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QRegularExpression>
#include <QDebug>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    display(nullptr),
    auxDisplay(nullptr),
    storedValue(0.0),
    pendingOp(),
    waitingForOperand(false)
{
    createWidgets();
    setWindowTitle("簡易計算機");
    setFixedSize(340, 420);
}

void MainWindow::createWidgets()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    auxDisplay = new QLabel("", this);
    auxDisplay->setAlignment(Qt::AlignRight);
    auxDisplay->setStyleSheet("color: gray; font-size: 12px; padding-right:6px;");

    display = new QLineEdit("0", this);
    display->setReadOnly(true);
    display->setAlignment(Qt::AlignRight);
    display->setMinimumHeight(60);
    display->setStyleSheet("font-size: 22px;");

    // buttons
    QString digits[10] = { "0","1","2","3","4","5","6","7","8","9" };

    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(6);

    // row 0: clear, backspace, sqrt, divide
    QPushButton *clearBtn = new QPushButton("C");
    QPushButton *backBtn = new QPushButton("←");
    QPushButton *sqrtBtn = new QPushButton("√");
    QPushButton *divBtn = new QPushButton("/");

    clearBtn->setMinimumSize(70, 50);
    backBtn->setMinimumSize(70, 50);
    sqrtBtn->setMinimumSize(70, 50);
    divBtn->setMinimumSize(70, 50);

    grid->addWidget(clearBtn, 0, 0);
    grid->addWidget(backBtn, 0, 1);
    grid->addWidget(sqrtBtn, 0, 2);
    grid->addWidget(divBtn, 0, 3);

    // digits positions:
    // row1: 1 2 3
    // row2: 4 5 6
    // row3: 7 8 9
    // row4: 0(span 2 cols) . =
    int positions[10][2] = {
        {4,0}, // 0 will be added with colspan 2
        {1,0}, {1,1}, {1,2}, // 1 2 3
        {2,0}, {2,1}, {2,2}, // 4 5 6
        {3,0}, {3,1}, {3,2}  // 7 8 9
    };

    // create digit buttons 1..9
    for (int i = 1; i <= 9; ++i) {
        QPushButton *btn = new QPushButton(digits[i]);
        btn->setMinimumSize(70, 60);
        grid->addWidget(btn, positions[i][0], positions[i][1]);
        connect(btn, &QPushButton::clicked, this, &MainWindow::digitClicked);
    }
    // zero (span 2 columns)
    QPushButton *zeroBtn = new QPushButton(digits[0]);
    zeroBtn->setMinimumHeight(60);
    zeroBtn->setMinimumWidth(70);
    grid->addWidget(zeroBtn, positions[0][0], positions[0][1], 1, 2); // colspan=2
    connect(zeroBtn, &QPushButton::clicked, this, &MainWindow::digitClicked);

    // point
    QPushButton *pointBtn = new QPushButton(".");
    pointBtn->setMinimumSize(70, 60);
    grid->addWidget(pointBtn, 4, 2);
    connect(pointBtn, &QPushButton::clicked, this, &MainWindow::pointClicked);

    // operators and =
    QPushButton *mulBtn = new QPushButton("*");
    QPushButton *subBtn = new QPushButton("-");
    QPushButton *addBtn = new QPushButton("+");
    QPushButton *eqBtn  = new QPushButton("=");

    mulBtn->setMinimumSize(70, 60);
    subBtn->setMinimumSize(70, 60);
    addBtn->setMinimumSize(70, 60);
    eqBtn->setMinimumSize(70, 60);

    grid->addWidget(mulBtn, 1, 3);
    grid->addWidget(subBtn, 2, 3);
    grid->addWidget(addBtn, 3, 3);
    grid->addWidget(eqBtn, 4, 3);

    // connect signals
    connect(subBtn, &QPushButton::clicked, this, &MainWindow::operatorClicked);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::operatorClicked);
    connect(mulBtn, &QPushButton::clicked, this, &MainWindow::operatorClicked);
    connect(divBtn, &QPushButton::clicked, this, &MainWindow::operatorClicked);

    connect(eqBtn, &QPushButton::clicked, this, &MainWindow::equalClicked);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::clearAll);
    connect(backBtn, &QPushButton::clicked, this, &MainWindow::backspace);
    connect(sqrtBtn, &QPushButton::clicked, this, &MainWindow::sqrtClicked);
    connect(pointBtn, &QPushButton::clicked, this, &MainWindow::pointClicked);
    connect(zeroBtn, &QPushButton::clicked, this, &MainWindow::digitClicked);

    // layout
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(auxDisplay);
    vLayout->addWidget(display);
    vLayout->addLayout(grid);
    central->setLayout(vLayout);

    updateAuxDisplay();
}

void MainWindow::digitClicked()
{
    QPushButton *clicked = qobject_cast<QPushButton*>(sender());
    if (!clicked) return;
    QString digit = clicked->text();

    if (display->text() == "0" && digit == "0")
        return;

    if (waitingForOperand) {
        display->clear();
        waitingForOperand = false;
    }

    if (display->text() == "0" && digit != ".")
        display->setText(digit);
    else
        display->setText(display->text() + digit);
}

void MainWindow::pointClicked()
{
    if (waitingForOperand) {
        display->setText("0");
        waitingForOperand = false;
    }
    if (!display->text().contains('.'))
        display->setText(display->text() + ".");
}

void MainWindow::operatorClicked()
{
    QPushButton *clicked = qobject_cast<QPushButton*>(sender());
    if (!clicked) return;
    QString op = clicked->text();

    double operand = display->text().toDouble();

    if (!pendingOp.isEmpty()) {
        if (!calculate(operand)) {
            abortOperation();
            return;
        }
        display->setText(QString::number(storedValue, 'g', 15));
    } else {
        storedValue = operand;
    }

    pendingOp = op;
    waitingForOperand = true;
    updateAuxDisplay();
}

void MainWindow::equalClicked()
{
    double operand = display->text().toDouble();

    if (!pendingOp.isEmpty()) {
        if (!calculate(operand)) {
            abortOperation();
            return;
        }
        display->setText(QString::number(storedValue, 'g', 15));
        pendingOp.clear();
        waitingForOperand = true;
        updateAuxDisplay();
    }
}

bool MainWindow::calculate(double rightOperand)
{
    if (pendingOp == "+") {
        storedValue += rightOperand;
    } else if (pendingOp == "-") {
        storedValue -= rightOperand;
    } else if (pendingOp == "*") {
        storedValue *= rightOperand;
    } else if (pendingOp == "/") {
        if (qFuzzyCompare(rightOperand + 1.0, 1.0)) { // rightOperand == 0
            return false; // division by zero
        }
        storedValue /= rightOperand;
    } else {
        return false;
    }
    return true;
}

void MainWindow::abortOperation()
{
    display->setText("Error");
    storedValue = 0;
    pendingOp.clear();
    waitingForOperand = true;
    updateAuxDisplay();
}

void MainWindow::clearAll()
{
    display->setText("0");
    storedValue = 0.0;
    pendingOp.clear();
    waitingForOperand = false;
    updateAuxDisplay();
}

void MainWindow::backspace()
{
    if (waitingForOperand) return;
    QString text = display->text();
    if (text.length() > 1) {
        text.chop(1);
        display->setText(text);
    } else {
        display->setText("0");
    }
}

void MainWindow::sqrtClicked()
{
    QString text = display->text();
    double val = text.toDouble();
    if (val < 0.0) {
        abortOperation();
        return;
    }
    double res = std::sqrt(val);
    display->setText(QString::number(res, 'g', 15));
    // after sqrt, treat as entering a new operand (does not clear pendingOp)
    waitingForOperand = false;
}

void MainWindow::updateAuxDisplay()
{
    if (!pendingOp.isEmpty()) {
        // show storedValue and pending operator, e.g. "123 +"
        auxDisplay->setText(QString("%1 %2").arg(QString::number(storedValue, 'g', 15), pendingOp));
    } else {
        auxDisplay->clear();
    }
}
