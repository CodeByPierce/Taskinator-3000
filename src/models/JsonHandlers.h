#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

enum Day {
    None = 0x0,
    Sun = 0x1,
    Mon  = 0x2,
    Tue  = 0x4,
    Wed  = 0x6,
    Thu  = 0x10,
    Fri = 0x20,
    Sat = 0x40
};

// Map between Qt's days and the dayFlag
inline const QMap<Qt::DayOfWeek, Day> dayMap = {
    {Qt::Sunday, Sun}
    , {Qt::Monday, Mon}
    , {Qt::Tuesday, Tue}
    , {Qt::Wednesday, Wed}
    , {Qt::Thursday, Thu}
    , {Qt::Friday, Fri}
    , {Qt::Saturday, Sat}
};

inline Day getFlagFromQtDay(int day) {
    return dayMap.value(static_cast<Qt::DayOfWeek>(day), None);
}

Q_DECLARE_FLAGS(Days, Day)
Q_DECLARE_OPERATORS_FOR_FLAGS(Days)

struct todoTask
{
    int id; // unique
    QString name;
    int completion; // A positive int between 0 and 100, representing completion percentage
    QDate dueDate;

    void fromJson(const QJsonObject &obj);
    QJsonObject toJson () const;
};

struct todoTaskList
{
    int nextId;
    QMap<int, todoTask> tasks;

    void fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;
};

struct todoOrder
{
    QList<int> order; // List of todoTask ids, the order represents the task order in the UI

    void fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;
};

struct dailyTask
{
    int id; // unique
    QString name;
    QDate creationDate;
    QDate deletionDate;
    Days weekly = Sun | Mon | Tue | Wed | Thu | Fri | Sat;

    void fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;
};

struct dailyTaskList
{
    QMap<int, dailyTask> tasks;
    int nextId;

    void fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;
};

struct dailyOrder
{
    QList<int> order; // List of dailyTask ids, the order represents the task order in the UI

    void fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;
};

struct dailyCompletion
{
    QDate date;
    QMap<int, int> taskCompletion; // <id, completion> 

    void fromJson(const QJsonObject &obj);
    QJsonObject toJson () const;
};

struct dailyCompletionMap
{
    QMap<QDate, dailyCompletion> dailyCompletions;
    // The json file should just be an array of {date, taskCompletion} object, the map is just for convenient programatic lookup

    void fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;
};

#endif