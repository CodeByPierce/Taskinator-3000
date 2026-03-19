#ifndef TODOTAB_H
#define TODOTAB_H

#include "todotask.h"

#include <QWidget>

namespace Ui {
class TodoTab;
}

class TodoTab : public QWidget
{
    Q_OBJECT

public:
    explicit TodoTab(QWidget *parent = nullptr);
    ~TodoTab();

private:
    Ui::TodoTab *ui;
    DataHandler& dataHandler;

    void addListItem(TodoTask* task, bool insertTop); // Used both for new list items as well as for loading from data
    void indexUpdate(); // Calling this ONLY updates the index of the TodoTask UI items
                        //    The reason for the difference is that, when creating a new task, underlying data order is handled automatically by the DataHandler,
                        //    so this avoids redundant data operations


    TodoTask* getTaskAt(int row) const;
    void loadFromData();
private slots:
    void createNewTask();
    void orderUpdate(); // This updates the order of the underlying data AND the index of the TodoTask UI Items
    void progSort();
    void dateSort();
    void removeTask(TodoTask* task); // Handles the case when a TodoTask needs to remove itself form the list
};

#endif // TODOTAB_H
