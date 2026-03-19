#include "todotab.h"
#include "ui_todotab.h"
#include <QInputDialog>


TodoTab::TodoTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TodoTab)
    , dataHandler(DataHandler::instance())
{
    ui->setupUi(this);

    connect(ui->pushButton_create, &QPushButton::clicked, this, &TodoTab::createNewTask);
    connect(ui->listWidget->model(), &QAbstractItemModel::rowsMoved, this, &TodoTab::orderUpdate);
    connect(ui->listWidget->model(), &QAbstractItemModel::layoutChanged, this, &TodoTab::orderUpdate);
    connect(ui->pushButton_progress, &QPushButton::clicked, this, &TodoTab::progSort);
    connect(ui->pushButton_urgent, &QPushButton::clicked, this, &TodoTab::dateSort);

    loadFromData();
}

TodoTab::~TodoTab()
{
    delete ui;
}

TodoTask* TodoTab::getTaskAt(int row) const {
    QListWidgetItem *item = ui->listWidget->item(row);
    if (!item) return nullptr;
    return qobject_cast<TodoTask*>(ui->listWidget->itemWidget(item));
}

void TodoTab::createNewTask() {
    bool ok;
    QString name = QInputDialog::getText(this,
                                         tr("New Task"),      // Window title
                                         tr("Task name:"),    // Label text
                                         QLineEdit::Normal,   // Input mode (Normal/Password)
                                         "",                  // Default text
                                         &ok);                // Status check

    if (ok) {
        // User clicked OK, empty text allowed
        TodoTask* task = new TodoTask(name, this);
        this->addListItem(task, true); // New task is inserted at the top of the list, hence true
    }

    indexUpdate();

}

void TodoTab::loadFromData() {
    TodoTask* task;
    for (int id : dataHandler.readTodoOrder()) {
        task = new TodoTask(id, this);
        this->addListItem(task, false); // Loaded tasks are inserted at the bottom of the list, starting with the first task, hence false
    }
}

void TodoTab::addListItem(TodoTask* task, bool insertTop) {
    QListWidgetItem *item = new QListWidgetItem();

    if (insertTop) {
        // Insert at the top (for new tasks)
        ui->listWidget->insertItem(0, item);
    } else {
        // Insert at the bottom (when loading tasks from storage)
        ui->listWidget->addItem(item);
    }

    item->setSizeHint(task->sizeHint());
    item->setForeground(Qt::transparent);
    ui->listWidget->setItemWidget(item, task);

    connect(task, &TodoTask::removeMe, this, &TodoTab::removeTask);
}

void TodoTab::indexUpdate() {
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        if (TodoTask *task = getTaskAt(i)) {
            task->updateIndex(i + 1);
        }
    }


}

void TodoTab::orderUpdate() {
    QList<int> order;
    int count = ui->listWidget->count();
    order.reserve(count);

    for (int i = 0; i < count; ++i) {
        if (TodoTask *task = getTaskAt(i)) {
            task->updateIndex(i + 1);
            order.append(task->getId());
        }
    }

    dataHandler.setTodoOrder(order);
}

void TodoTab::progSort() {
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        QWidget *widget = ui->listWidget->itemWidget(item);
        TodoTask *task = qobject_cast<TodoTask*>(widget);

        item->setData(Qt::DisplayRole, task->getProgLexSortVal());
    }

     ui->listWidget->sortItems(Qt::AscendingOrder);
}

void TodoTab::dateSort() {
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        QWidget *widget = ui->listWidget->itemWidget(item);
        TodoTask *task = qobject_cast<TodoTask*>(widget);

        item->setData(Qt::DisplayRole, task->getDateLexSortVal());
    }

    ui->listWidget->sortItems(Qt::AscendingOrder);
}

void TodoTab::removeTask(TodoTask* task) {
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        if (ui->listWidget->itemWidget(item) == task) {

            delete ui->listWidget->takeItem(i);

            return;
        }
    }
}