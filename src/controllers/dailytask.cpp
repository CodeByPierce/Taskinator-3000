#include "dailytask.h"
#include "ui_dailytask.h"

DailyTask::DailyTask(const int id, const QDate &currentDate, bool hidden, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DailyTask)
    , dataHandler(DataHandler::instance())
    , id(id)
    , currentDate(currentDate)
{
    ui->setupUi(this);

    if (hidden) {
        this->hide();
        // This represent daily tasks which are not shown for the given day
        // They are still created for the sake of convenient edits to the daily task order,
        // without disrupting the continuity of tasks that are not shown for the given day
        // They don't need a functioning UI
    } else {
        setup();
    }
}

DailyTask::DailyTask(const QString &name, const QDate &currentDate, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DailyTask)
    , dataHandler(DataHandler::instance())
    , currentDate(currentDate)
{
    id = dataHandler.createDailyTask(name, currentDate);
    ui->setupUi(this);
    setup();
}

DailyTask::~DailyTask()
{
    delete ui;
}

void DailyTask::setup() {

    buttonMap = {
        {Sun, ui->pushButton_sun},
        {Mon, ui->pushButton_mon},
        {Tue, ui->pushButton_tue},
        {Wed, ui->pushButton_wed},
        {Thu, ui->pushButton_thu},
        {Fri, ui->pushButton_fri},
        {Sat, ui->pushButton_sat}
    };

    connect(ui->lineEdit_name, &QLineEdit::editingFinished, this, &DailyTask::nameSet);
    connect(ui->progressBar, &QProgressBar::valueChanged, this, &DailyTask::progPeripheralsUpdate);
    connect(ui->pushButton_dec, &QPushButton::clicked, this, &DailyTask::progressDec);
    connect(ui->pushButton_inc, &QPushButton::clicked, this, &DailyTask::progressInc);

    // Connect all the day buttons to the ol' updater slot
    for (const auto bttn : buttonMap) {
        connect(bttn, &QPushButton::clicked, this, &DailyTask::weeklySet);
    }

    const dailyTask d = dataHandler.readDailyTaskById(id);

    // Set default/loaded name info
    ui->lineEdit_name->setPlaceholderText("Set Task Name");
    ui->lineEdit_name->setText(d.name);

    // Set default/loaded progress value
    ui->progressBar->setValue(dataHandler.readCompletionByDateId(currentDate, id));

    // Set all the buttons to the correct checked state
    Days weekly = d.weekly;
    for (const auto &[day, bttn] : buttonMap.asKeyValueRange()) {
        bttn->setChecked(weekly.testFlag(day));
    }

    // For all the buttons in the list, avoid weird focus bugs when clicking them
    QList<QPushButton *> fixFocusBug = buttonMap.values();
    fixFocusBug += {ui->pushButton_inc, ui->pushButton_dec};
    for (auto widget : fixFocusBug) {
        widget->setFocusPolicy(Qt::NoFocus);
    }

}

void DailyTask::weeklySet() {
    Days weekly;
    for (const auto &[day, bttn] : buttonMap.asKeyValueRange()) {
        if (bttn->isChecked()) {
            weekly.setFlag(day);
        }
    }
    dataHandler.setDailyTaskDays(id, weekly);

    // Remove widget from the current day if it's no longer active this day
    if (!weekly.testFlag(getFlagFromQtDay(currentDate.dayOfWeek()))) {
        emit removeMe(this);
    }
}

void DailyTask::nameSet() {
  dataHandler.setDailyTaskName(id, ui->lineEdit_name->text());
}

void DailyTask::progPeripheralsUpdate(int value) {
    // Update decrement button appearance
    if (value <= 0) {
        ui->pushButton_dec->setIcon(QIcon());
        ui->pushButton_dec->setText("Remove");
    } else {
        ui->pushButton_dec->setIcon(QIcon::fromTheme("list-remove"));
        ui->pushButton_dec->setText("");
    }
}

void DailyTask::progressInc() {
    setProgress(ui->progressBar->value() + this->progIncSize);
}

void DailyTask::progressDec() {
    int progress = ui->progressBar->value();
    if (progress <= 0) {
        // The task WAS at 0 and the user clicked "remove"
        deleteTask();
    } else {
        setProgress(progress - this->progIncSize);
    }
}

void DailyTask::setProgress(int progress) {
    ui->progressBar->setValue(progress);
    dataHandler.setDailyTaskCompletion(id, currentDate, progress);
    emit progressUpdated();
}

void DailyTask::deleteTask() {
    // remove task from database
    dataHandler.deleteDailyTask(id, currentDate);
    emit removeMe(this);
    return;
}

const QString DailyTask::getProgLexSortVal() {
    int progress = dataHandler.readCompletionByDateId(currentDate, id);
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