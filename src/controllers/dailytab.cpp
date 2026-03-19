#include "dailytab.h"
#include "ui_dailytab.h"

#include <QInputDialog>

DailyTab::DailyTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DailyTab)
    , dataHandler(DataHandler::instance())
{
    ui->setupUi(this);

    // Create the calendar popup for the date field and setup the event filter
    calendar = new QCalendarWidget(this);
    calendar->setWindowFlags(Qt::Popup);
    ui->pushButton_pick->installEventFilter(this);

    connect(calendar, &QCalendarWidget::clicked, this, &DailyTab::gotoDate);
    connect(ui->pushButton_create, &QPushButton::clicked, this, &DailyTab::createNewTask);
    connect(ui->listWidget->model(), &QAbstractItemModel::rowsMoved, this, &DailyTab::orderUpdate);
    connect(ui->listWidget->model(), &QAbstractItemModel::layoutChanged, this, &DailyTab::orderUpdate);
    connect(ui->pushButton_progress, &QPushButton::clicked, this, &DailyTab::progSort);
    connect(ui->pushButton_today, &QPushButton::clicked, this, &DailyTab::gotoToday);
    connect(ui->pushButton_tomorrow, &QPushButton::clicked, this, &DailyTab::gotoTomorrow);
    connect(ui->pushButton_yesterday, &QPushButton::clicked, this, &DailyTab::gotoYesterday);

    loadDayFromData(QDate::currentDate());
}

DailyTab::~DailyTab()
{
    delete ui;
}

DailyTask* DailyTab::getTaskAt(int row) const {
    QListWidgetItem *item = ui->listWidget->item(row);
    if (!item) return nullptr;
    return qobject_cast<DailyTask*>(ui->listWidget->itemWidget(item));
}

void DailyTab::createNewTask() {
    bool ok;
    QString name = QInputDialog::getText(this,
                                         tr("New Task"),      // Window title
                                         tr("Task name:"),    // Label text
                                         QLineEdit::Normal,   // Input mode (Normal/Password)
                                         "",                  // Default text
                                         &ok);                // Status check

    if (ok) {
        // User clicked OK, empty text allowed
        DailyTask* task = new DailyTask(name, currentDisplayDate, this);
        this->addListItem(task, true, false); // New task is inserted at the top of the list, not hidden, hence true, false
    }
    updateMasterProgressBar();
}

void DailyTab::loadDayFromData(const QDate& currentDate) {
    // Set the member variable for the current date
    this->currentDisplayDate = currentDate;

    // Update the date display
    ui->label_date->setText(currentDate.toString(this->dateFormat));

    // Unload any items in the current list widget
    // Momentarily disable signaling, so that "layoutChanged" doesn't cause the underlying task order to get modified
    ui->listWidget->blockSignals(true);
    ui->listWidget->clear();
    ui->listWidget->blockSignals(false);
    ui->listWidget->update(); // This is a just-in-case, to make sure the widget is visually updated to represent the wipe

    // Load in every dailyTask, hiding the ones that either don't exist or are inacitve for this day
    DailyTask *task;
    for (int id : dataHandler.readDailyOrder()) {
        dailyTask d = dataHandler.readDailyTaskById(id);

        // If the asnwer to each of the following is "Yes", then the task is displayed on this day, otherwise it's created as hidden
        //    - Was the task created on or before today?
        //    - Has the task either not been deleted, or deleted AFTER today?
        //    - Is the task active on this day of the week?
        if (currentDate.daysTo(d.creationDate) <= 0
            && (d.deletionDate.isNull() || currentDate.daysTo(d.deletionDate) > 0)
            && d.weekly.testFlag(getFlagFromQtDay(currentDate.dayOfWeek()))
        )
        {
            task = new DailyTask(id, currentDate, false); // Not hidden, hence false
            addListItem(task, false, false); // Inserted at the bottom, not hidden, hence false, false
        } else {
            task = new DailyTask(id, currentDate, true); // Hidden, hence true
            addListItem(task, false, true); // Inserted at the bottom, hidden, hence false, true
        }
    }

    // Once all tasks have been loaded in, update the progress bar at the top
    updateMasterProgressBar();
}

void DailyTab::addListItem(DailyTask* task, bool insertTop, bool hidden) {
    QListWidgetItem *item = new QListWidgetItem();

    if (insertTop) {
        // Insert at the top (for new tasks)
        ui->listWidget->insertItem(0, item);
    } else {
        // Insert at the bottom (when loading tasks from storage)
        ui->listWidget->addItem(item);
    }

    if (hidden) {
        item->setHidden(true);
    } else {
        item->setSizeHint(task->sizeHint());
        item->setForeground(Qt::transparent);
        connect(task, &DailyTask::removeMe, this, &DailyTab::removeTask);
        connect(task, &DailyTask::progressUpdated, this, &DailyTab::updateMasterProgressBar);
    }
    ui->listWidget->setItemWidget(item, task);

}

void DailyTab::orderUpdate() {
    QList<int> order;
    int count = ui->listWidget->count();
    order.reserve(count);

    for (int i = 0; i < count; ++i) {
        if (DailyTask *task = getTaskAt(i)) {
            order.append(task->getId());
        }
    }

    dataHandler.setDailyOrder(order);
}

void DailyTab::progSort() {
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        QWidget *widget = ui->listWidget->itemWidget(item);
        DailyTask *task = qobject_cast<DailyTask*>(widget);

        item->setData(Qt::DisplayRole, task->getProgLexSortVal());
    }

    ui->listWidget->sortItems(Qt::AscendingOrder);
}

void DailyTab::removeTask(DailyTask* task) {
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        if (ui->listWidget->itemWidget(item) == task) {

            delete ui->listWidget->takeItem(i);
            updateMasterProgressBar();
            return;
        }
    }
}

void DailyTab::gotoToday() {
    loadDayFromData(QDate::currentDate());
}

void DailyTab::gotoTomorrow() {
    loadDayFromData(currentDisplayDate.addDays(1));
}

void DailyTab::gotoYesterday() {
    loadDayFromData(currentDisplayDate.addDays(-1));
}

void DailyTab::gotoDate(const QDate& date) {
    calendar->hide();
    loadDayFromData(date);
}

void DailyTab::updateMasterProgressBar() {
    double progress = 0;
    int numRowsShown = 0;
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        if(!(item->isHidden())) {
            QWidget *widget = ui->listWidget->itemWidget(item);
            DailyTask *task = qobject_cast<DailyTask*>(widget);

            progress += task->getProgress();
            numRowsShown++;
        }
    }
    if (numRowsShown > 0) {
        progress = progress / numRowsShown;
    }
    ui->progressBar->setValue(qRound(progress));
}

bool DailyTab::eventFilter(QObject *obj, QEvent *event) {

    // Catch and intercept click on the pick button
    if (obj == ui->pushButton_pick && event->type() == QEvent::MouseButtonPress) {
        // Show calendar at the button's position
        QPoint globalPos = ui->pushButton_pick->mapToGlobal(ui->pushButton_pick->rect().bottomRight());
        calendar->move(globalPos);
        calendar->show();
        return true; // Returning true prevents the click from being porocessed elsewhere
    }
    // Pass on all other events to the base class for default handling
    return QWidget::eventFilter(obj, event);
}