#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dailytab.h"
#include "todotab.h"

#include <QMainWindow>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    DailyTab *m_dailyTab;
    TodoTab *m_todoTab;
    DataHandler& dataHandler;
};

#endif // MAINWINDOW_H
