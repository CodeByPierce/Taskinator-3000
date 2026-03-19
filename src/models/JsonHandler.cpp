#include "JsonHandlers.h"

// If you change variable names, the JSON files won't be backwards compatible
#define TO_JSON(obj, var) obj[#var] = this->var
#define FROM_JSON_STR(obj, var) this->var = obj[#var].toString("Something went wrong")
#define FROM_JSON_INT(obj, var) this->var = obj[#var].toInt(-1)
#define FROM_JSON_BOOL(obj, var) this->var = obj[#var].toBool()

// --- todoTask --- //

void todoTask::fromJson(const QJsonObject &obj) {
    FROM_JSON_INT(obj, id);
    FROM_JSON_STR(obj, name);
    FROM_JSON_INT(obj, completion);
    dueDate = QDate::fromString(obj["dueDate"].toString(), Qt::ISODate);
}

QJsonObject todoTask::toJson () const {
    QJsonObject obj;
    TO_JSON(obj, id);
    TO_JSON(obj, name);
    TO_JSON(obj, completion);
    obj["dueDate"] = dueDate.toString(Qt::ISODate);

    return obj;
}

// --- todoTaskList --- //

void todoTaskList::fromJson(const QJsonObject &obj) {
    FROM_JSON_INT(obj, nextId);
    tasks.clear();
    QJsonArray arr = obj["tasks"].toArray();

    for (const QJsonValue &value : arr) {
        todoTask newTask;
        newTask.fromJson(value.toObject());
        tasks[newTask.id] = newTask;
    }
}

QJsonObject todoTaskList::toJson() const {
    QJsonObject obj;
    QJsonArray arr;
    TO_JSON(obj, nextId);

    for (const todoTask &task : tasks) {
        arr.append(task.toJson());
    }

    obj["tasks"] = arr;

    return obj;
}

// --- todoOrder --- //

void todoOrder::fromJson(const QJsonObject &obj) {
    order.clear();
    QJsonArray arr = obj["order"].toArray();

    for (const QJsonValue &value : arr) {
        order.append(value.toInt(-1));
    }
}

QJsonObject todoOrder::toJson() const {
    QJsonObject obj;
    QJsonArray arr;

    for (const int i : order) {
        arr.append(i);
    }

    obj["order"] = arr;
    return obj;
}

// --- dailyTask --- //

void dailyTask::fromJson(const QJsonObject &obj) {
    FROM_JSON_INT(obj, id);
    FROM_JSON_STR(obj, name);
    creationDate = QDate::fromString(obj["creationDate"].toString(), Qt::ISODate);
    deletionDate = QDate::fromString(obj["deletionDate"].toString(), Qt::ISODate);
    weekly = Days::fromInt(obj["weekly"].toInt(-1));

}

QJsonObject dailyTask::toJson() const {
    QJsonObject obj;
    TO_JSON(obj, id);
    TO_JSON(obj, name);
    obj["creationDate"] = creationDate.toString(Qt::ISODate);
    obj["deletionDate"] = deletionDate.toString(Qt::ISODate);
    obj["weekly"] = static_cast<int>(weekly);

    return obj;

}

// --- dailyTaskList --- //

void dailyTaskList::fromJson(const QJsonObject &obj) {
    FROM_JSON_INT(obj, nextId);
    tasks.clear();
    QJsonArray arr = obj["tasks"].toArray();

    for (const QJsonValue &value : arr) {
        dailyTask newTask;
        newTask.fromJson(value.toObject());
        tasks[newTask.id] = newTask;
    }
}

QJsonObject dailyTaskList::toJson() const {
    QJsonObject obj;
    QJsonArray arr;
    TO_JSON(obj, nextId);

    for (const dailyTask &task : tasks) {
        arr.append(task.toJson());
    }

    obj["tasks"] = arr;

    return obj;
}


// --- dailyOrder --- //

void dailyOrder::fromJson(const QJsonObject &obj) {
    order.clear();
    QJsonArray arr = obj["order"].toArray();

    for (const QJsonValue &value : arr) {
        order.append(value.toInt(-1));
    }
}

QJsonObject dailyOrder::toJson() const {
    QJsonObject obj;
    QJsonArray arr;

    for (const int i : order) {
        arr.append(i);
    }

    obj["order"] = arr;
    return obj;
}


// --- dailyCompletion --- //

void dailyCompletion::fromJson(const QJsonObject &obj) {
    QJsonArray arr;
    taskCompletion.clear();
    date = QDate::fromString(obj["date"].toString(), Qt::ISODate);
    arr = obj["taskCompletion"].toArray();
    for (const QJsonValue &value : arr) {
        QJsonObject tmp = value.toObject();
        taskCompletion[tmp["id"].toInt(-1)] = tmp["completion"].toInt(-1);
    } 
}

QJsonObject dailyCompletion::toJson () const {
    QJsonObject obj;
    QJsonArray arr;
    obj["date"] = date.toString(Qt::ISODate);

    for ( const auto [_id, _completion] : taskCompletion.asKeyValueRange()) {
        QJsonObject tmp;
        tmp["id"] = _id;
        tmp["completion"] = _completion;
        arr.append(tmp);
    }
    obj["taskCompletion"] = arr;
    return obj;
}

// --- dailyCompletionMap --- //

void dailyCompletionMap::fromJson(const QJsonObject &obj) {
    QJsonArray arr;
    dailyCompletions.clear();
    arr = obj["dailyCompletions"].toArray();

    for (const QJsonValue &value : arr) {
        dailyCompletion comp;
        comp.fromJson(value.toObject());
        dailyCompletions[comp.date] = comp;
    }
}

QJsonObject dailyCompletionMap::toJson() const {
    QJsonObject obj;
    QJsonArray arr;

    for ( const dailyCompletion &comp : dailyCompletions) {
        arr.append(comp.toJson());
    }

    obj["dailyCompletions"] = arr;
    return obj;
}