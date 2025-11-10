#pragma once

#include <QMainWindow>
#include <QString>

class QLineEdit;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void digitClicked();
    void pointClicked();
    void operatorClicked();
    void equalClicked();
    void clearAll();
    void backspace();
    void sqrtClicked(); // 新增：根號按鈕處理

private:
    void createWidgets();
    bool calculate(double rightOperand);
    void abortOperation();      // 已新增：避免紅線
    void updateAuxDisplay();    // 顯示 storedValue + pendingOp

    QLineEdit *display;
    QLabel *auxDisplay; // 新增：顯示目前運算子/暫存值
    double storedValue;
    QString pendingOp;
    bool waitingForOperand;
};
