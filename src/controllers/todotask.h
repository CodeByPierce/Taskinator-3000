#ifndef TODOTASK_H
#define TODOTASK_H

#include "DataHandler.h"

#include <QWidget>
#include <QCalendarWidget>

namespace Ui {
class TodoTask;
}

class TodoTask : public QWidget
{
    Q_OBJECT

public:
    explicit TodoTask(const int id, QWidget *parent = nullptr);     // When loading the UI from JSON files
    explicit TodoTask(const QString &name, QWidget *parent = nullptr);    // When creating a new task in the UI
    ~TodoTask();

    int getId() const {return id;}
    const QString getProgLexSortVal();
    const QString getDateLexSortVal();

    void updateIndex(int index);

protected:
    // Use this to intercept the date line edit and replace with a calendar popup
    bool eventFilter(QObject *obj, QEvent *event) override;


private:
    Ui::TodoTask *ui;
    DataHandler& dataHandler;
    QCalendarWidget *calendar; // Used to implement the popup for the date lineEdit
    int id;

    const QString dateFormat = "M/dd/yyyy"; // Format displayed for task due dates
    const int progIncSize = 10; // The increment (and decrement) size to use for the progress bar. 10 means 10%

    void setup(); // For all the constructor stuff that doesn't need to be duplicated in each constructor
    void deleteTask(); // Only call this when you want to delete the task from the database itself! Not for simply cleaning up the UI
    void setProgress(int progress); // Only call this when you want to change the underlying data!

private slots:
    void nameSet(); // Changes underlying data
    void dateSet(const QDate &date); // Changes underlying data
    void dateCleared();
    void datePeripheralsUpdate(const QString &text);
    void progPeripheralsUpdate(int value);
    void progressInc();
    void progressDec();

signals:
    void removeMe(TodoTask* task);

};

#endif // TODOTASK_H
