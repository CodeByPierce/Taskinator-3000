#ifndef DAILYTAB_H
#define DAILYTAB_H

#include "dailytask.h"

#include <QWidget>

namespace Ui {
class DailyTab;
}

class DailyTab : public QWidget
{
    Q_OBJECT

public:
    explicit DailyTab(QWidget *parent = nullptr);
    ~DailyTab();

protected:
    // Use this to intercept the date line edit and replace with a calendar popup
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::DailyTab *ui;
    DataHandler& dataHandler;
    QDate currentDisplayDate; // Whatever the current date in the UI is
    QCalendarWidget *calendar; // Used to implement the popup for the Pick Date button

    const QString dateFormat = "dddd, MMM dd, yyyy"; // Format displayed for current day

    void addListItem(DailyTask* task, bool insertTop, bool hidden); // Used both for new list items as well as for loading from data
    DailyTask* getTaskAt(int row) const;
    void loadDayFromData(const QDate &currentDate);

private slots:
    void createNewTask();
    void orderUpdate(); // This updates the order of the underlying data
    void progSort();
    void removeTask(DailyTask* task); // Handles the case when a DailyTask needs to remove itself form the list
    void gotoToday();
    void gotoYesterday();
    void gotoTomorrow();
    void gotoDate(const QDate& date);
    void updateMasterProgressBar();
};

#endif // DAILYTAB_H
