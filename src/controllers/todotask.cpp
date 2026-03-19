#include "todotask.h"
#include "ui_todotask.h"

#include <QAbstractButton>

TodoTask::TodoTask(const int id, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TodoTask)
    , dataHandler(DataHandler::instance())
    , id(id)
{
    ui->setupUi(this);
    setup();
}

TodoTask::TodoTask(const QString &name, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TodoTask)
    , dataHandler(DataHandler::instance())
{
    this->id = dataHandler.createTodoTask(name);

    ui->setupUi(this);
    setup();

}

TodoTask::~TodoTask()
{
    delete ui;
}

void TodoTask::setup() {
    const todoTask t = dataHandler.readTodoTaskById(id);

    // Create the calendar popup for the date field and setup the event filter
    calendar = new QCalendarWidget(this);
    calendar->setWindowFlags(Qt::Popup);
    ui->lineEdit_date->installEventFilter(this);

    connect(calendar, &QCalendarWidget::clicked, this, &TodoTask::dateSet);
    connect(ui->lineEdit_name, &QLineEdit::editingFinished, this, &TodoTask::nameSet);
    connect(ui->pushButton_clear, &QPushButton::clicked, this, &TodoTask::dateCleared);
    connect(ui->lineEdit_date, &QLineEdit::textChanged, this, &TodoTask::datePeripheralsUpdate);
    connect(ui->progressBar, &QProgressBar::valueChanged, this, &TodoTask::progPeripheralsUpdate);
    connect(ui->pushButton_dec, &QPushButton::clicked, this, &TodoTask::progressDec);
    connect(ui->pushButton_inc, &QPushButton::clicked, this, &TodoTask::progressInc);

    // Set default/loaded name info
    ui->lineEdit_name->setPlaceholderText("Set Task Name");
    ui->lineEdit_name->setText(t.name);

    // Set default/loaded date info
    ui->lineEdit_date->setPlaceholderText("Set Due Date");
    if (t.dueDate.isNull()) {
        ui->lineEdit_date->setText("");
    } else {
        ui->lineEdit_date->setText(t.dueDate.toString(this->dateFormat));
    }

    // Set default/loaded progress value
    ui->progressBar->setValue(t.completion);

    // For all the buttons in the list, avoid weird focus bugs when clicking them
    QList<QWidget *> fixFocusBug = {ui->pushButton_clear, ui->pushButton_inc, ui->pushButton_dec};
    for (auto widget : fixFocusBug) {
        widget->setFocusPolicy(Qt::NoFocus);
    }


    // For all the UI elements in the list, retain their size when they are hidden
    QList<QWidget *> retainWhenHidden = {ui->pushButton_clear, ui->label_urgent};
    for (auto widget : retainWhenHidden) {
        QSizePolicy sp_retain = widget->sizePolicy();
        sp_retain.setRetainSizeWhenHidden(true);
        widget ->setSizePolicy(sp_retain);
    }


}

void TodoTask::nameSet() {
    dataHandler.setTodoTaskName(id, ui->lineEdit_name->text());
}

bool TodoTask::eventFilter(QObject *obj, QEvent *event) {

    // Catch and intercept click on the date linEdit
    if (obj == ui->lineEdit_date && event->type() == QEvent::MouseButtonPress) {
        // Show calendar at the LineEdit's position
        QPoint globalPos = ui->lineEdit_date->mapToGlobal(ui->lineEdit_date->rect().bottomLeft());
        calendar->move(globalPos);
        calendar->show();
        return true; // Returning true prevents the click from being porocessed elsewhere
    }
    // Pass on all other events to the base class for default handling
    return QWidget::eventFilter(obj, event);
}

void TodoTask::dateSet(const QDate &date) {

    if (date.isNull()) {
        ui->lineEdit_date->setText("");
    } else {
        ui->lineEdit_date->setText(date.toString(this->dateFormat));
    }

    calendar->hide();
    dataHandler.setTodoTaskDate(id, date);
}

void TodoTask::dateCleared() {
    QDate date; // Set an empty date
    dateSet(date);
}

void TodoTask::datePeripheralsUpdate(const QString &text) {
    if (text.isEmpty()) {
        ui->pushButton_clear->hide();
        ui->label_urgent->hide();
    } else {
        ui->pushButton_clear->show();

        // Put a red (1 week out) or yellow (one month out) exclamation point if the due date is soon.
        int daysUntil = QDate::currentDate().daysTo(QDate::fromString(text, this->dateFormat));
        if (daysUntil <= 30) {
            ui->label_urgent->show();
            ui->label_urgent->setText("!");

            if (daysUntil <= 7) {
                ui->label_urgent->setStyleSheet("font-size: 22pt; font-weight: bold; color: red;");
            } else {
                ui->label_urgent->setStyleSheet("font-size: 18pt; font-weight: bold; color: orange;");
            }
        } else {
            ui->label_urgent->hide();
        }
    }
}

void TodoTask::progPeripheralsUpdate(int value) {
    // Update increment button appearance
    if (value >= 100) {
        ui->pushButton_inc->setIcon(QIcon());
        ui->pushButton_inc->setText("Finish");
    } else {
        ui->pushButton_inc->setIcon(QIcon::fromTheme("list-add"));
        ui->pushButton_inc->setText("");
    }

    // Update decrement button appearance
    if (value <= 0) {
        ui->pushButton_dec->setIcon(QIcon());
        ui->pushButton_dec->setText("Remove");
    } else {
        ui->pushButton_dec->setIcon(QIcon::fromTheme("list-remove"));
        ui->pushButton_dec->setText("");
    }
}

void TodoTask::progressInc() {
    int progress = ui->progressBar->value();

    if (progress >= 100) {
        // The task WAS at 100 and the user clicked "Finish"
        deleteTask();
    } else {
        setProgress(progress + this->progIncSize);
    }
}

void TodoTask::progressDec() {
    int progress = ui->progressBar->value();

    if (progress <= 0) {
        // The task WAS at 0 and the user clicked "remove"
        deleteTask();
    } else {
        setProgress(progress - this->progIncSize);
    }
}

void TodoTask::setProgress(int progress) {
    ui->progressBar->setValue(progress);
    dataHandler.setTodoTaskCompletion(id, progress);
}

void TodoTask::deleteTask() {
    // remove task from database
    dataHandler.deleteTodoTask(id);
    emit removeMe(this);
    return;
}

void TodoTask::updateIndex(int index) {
    ui->label_index->setText(QString::number(index));
}

const QString TodoTask::getDateLexSortVal() {
    QDate date = dataHandler.readTodoTaskById(id).dueDate;
    if (date.isNull()) {
        return "9999-99-99";
    } else {
        return date.toString(Qt::ISODate);
    }

}

const QString TodoTask::getProgLexSortVal() {
    int progress = dataHandler.readTodoTaskById(id).completion;
    int value;

    if (progress <= 0) {
        value = 9999;
    } else if (progress >= 100) {
        value = 999;
    } else {
        value = 100 - progress;
    }

    return QString("%1").arg(value, 2, 10, QChar('0'));
}