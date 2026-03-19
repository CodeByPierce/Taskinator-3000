#ifndef DAILYTASK_H
#define DAILYTASK_H

#include "DataHandler.h"

#include <QWidget>
#include <QCalendarWidget>
#include <QPushButton>

namespace Ui {
class DailyTask;
}

class DailyTask : public QWidget
{
    Q_OBJECT

public:
    explicit DailyTask(const int id, const QDate &currentDate, bool hidden, QWidget *parent = nullptr);
    explicit DailyTask(const QString &name, const QDate &currentDate, QWidget *parent = nullptr);
    ~DailyTask();

    int getId() const {return id;}
    int getProgress() const {return dataHandler.readCompletionByDateId(currentDate, id);}
    const QString getProgLexSortVal();

private:
    Ui::DailyTask *ui;
    DataHandler& dataHandler;

    // Although the id is enough to identify dailyTasks in the database (which are agnostic of date)
    // DailyTask elements in the UI are also associated with a particular date,
    // since they are created and destroyed each time a day is loaded
    int id;
    QDate currentDate;

    QMap<Day, QPushButton *> buttonMap; // Associate all the Day flag types with their buttons, very useful!
                                        // Instantiated during setup, since it relies on ui pointers

    const int progIncSize = 10; // The increment (and decrement) size to use for the progress bar. 10 means 10%

    void setup();
    void deleteTask(); // Only call this when you want to affect the database itself! Not for simply cleaning up the UI
    void setProgress(int progress); // Only call this when you want to change the underlying data!

signals:
    // These 2 signals are specifically for the parent class (a list widget) to catch and use for updates
    void removeMe(DailyTask *task);
    void progressUpdated();


private slots:
    void nameSet(); // Changes underlying data
    void weeklySet(); // Changes underlying weekly flags
    void progressInc();
    void progressDec();
    void progPeripheralsUpdate(int value);
};

#endif // DAILYTASK_H
