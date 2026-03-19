#include "DataHandler.h"
#include <QStandardPaths>
#include <QDir>

QString DataHandler::getStorageFilePath(const QString &fileName) const {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    // Checks if the local storage folder exists already, creating it if it deosn't
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    return dir.filePath(fileName);
}

void DataHandler::loadAll() {
    // Return values ignored, better error handling desired here
    loadFromJsonFile(tTasks, tTasksFilename);
    loadFromJsonFile(tOrder, tOrderFilename);
    loadFromJsonFile(dTasks, dTasksFilename);
    loadFromJsonFile(dOrder, dOrderFilename);
    loadFromJsonFile(dCompletion, dCompletionFilename);
}

void DataHandler::saveAll() {
    // Return values ignored, better error handling desired here
    saveToJsonFile(tTasks, tTasksFilename);
    saveToJsonFile(tOrder, tOrderFilename);
    saveToJsonFile(dTasks, dTasksFilename);
    saveToJsonFile(dOrder, dOrderFilename);
    saveToJsonFile(dCompletion, dCompletionFilename);
}

int DataHandler::createTodoTask(const QString &name) {
    todoTask newTask;
    int newId = tTasks.nextId;
    newTask.id = newId;
    tTasks.nextId++;
    newTask.name = name;
    newTask.completion = 0;

    tTasks.tasks[newId] = newTask;
    tOrder.order.prepend(newId);

    return newId;
}

// TODO:
// Handle the case where look up id is not found
// Ensure order QList is the same size as before (enforcing strict reorder operations)
// Use .find to re-enable references in return types, for SPEEED

const todoTask DataHandler::readTodoTaskById(const int id) const {
    return tTasks.tasks.value(id);
}

const dailyTask DataHandler::readDailyTaskById(const int id) const {
    return dTasks.tasks.value(id);
}

void DataHandler::setTodoTaskName(const int id, const QString &name) {
    tTasks.tasks[id].name = name;
}

void DataHandler::setDailyTaskName(const int id, const QString &name) {
    dTasks.tasks[id].name = name;
}

void DataHandler::setTodoTaskDate(const int id, const QDate &dueDate) {
    tTasks.tasks[id].dueDate = dueDate;
}

void DataHandler::setTodoTaskCompletion(const int id, const int completion) {
    tTasks.tasks[id].completion = completion;
}

void DataHandler::setTodoOrder(const QList<int> &order) {
    tOrder.order = order;
}

void DataHandler::deleteTodoTask(const int id) {
    tOrder.order.removeOne(id);
    tTasks.tasks.remove(id);

}

const QList<int> &DataHandler::readTodoOrder() const{
    return tOrder.order;
}

int DataHandler::createDailyTask(const QString &name, const QDate &creationDate) {
    dailyTask newTask;
    int newId = dTasks.nextId;
    newTask.id = newId;
    dTasks.nextId++;
    newTask.name = name;
    newTask.creationDate = creationDate;

    dTasks.tasks[newId] = newTask;
    dOrder.order.prepend(newId);

    return newId;
}

// One of the key pieces of logic here is that we don't store 0% completion in the data
// The representation for 0% completion is that there is no data!
// This keeps our data files lean! (although somewhat less explicit)
void DataHandler::setDailyTaskCompletion(const int id, const QDate &date, const int completion) {
    QMap<QDate, dailyCompletion>& outerMap = dCompletion.dailyCompletions;
    auto outerIt = outerMap.find(date);
    if (outerIt != outerMap.end()) {
        // Outer Key found
        QMap<int, int>& innerMap = outerIt.value().taskCompletion;
        auto innerIt = innerMap.find(id);
        if (innerIt != innerMap.end()) {
            // Inner Key found
            if (!completion) {
                // So we want to delete this entry from the innerMap!
                innerMap.erase(innerIt);

                // And now if the innerMap is empty, we can delete the entry in the outerMap
                // This signifies that every task on this day is now at 0%
                if (innerMap.isEmpty()) {
                    outerMap.erase(outerIt);
                }
            } else {
                // Update the completion (previously non 0%) to some non 0% value
                innerIt.value() = completion;
            }
        } else {
            // Inner Key not found
            if (!completion) {
                return; // No key existed, and we don't make one fo 0% completion
            } else {
                // Now we insert a new key,
                // as this signifies the task on this day going from 0% to more than 0%
                innerMap.insert(innerIt, id, completion);
            }
        }
    } else {
        // Outer Key not found
        if (!completion){
            return; // nothing to change here!
        } else {
            // Now we need to create a new struct to represent the day's tasks
            // This signifies the first task on this day to go above 0%
            dailyCompletion newDComp;
            newDComp.date = date;
            newDComp.taskCompletion.insert(id, completion);

            // Now insert the struct into the map for the current date
            outerMap.insert(date, newDComp);
        }
    }
}

int DataHandler::readCompletionByDateId(const QDate &date, const int id) const {
    const QMap<QDate, dailyCompletion> &outerMap = dCompletion.dailyCompletions;
    auto outerIt = outerMap.find(date);
    if (outerIt != outerMap.end()) {
        // Outer Key found
        const QMap<int, int>& innerMap = outerIt.value().taskCompletion;
        auto innerIt = innerMap.find(id);
        if (innerIt != innerMap.end()) {
            // Inner Key found
            return innerIt.value();
        } else {
            // Inner Key not found
            return 0;
        }
    } else {
        // Outer Key not found
        return 0;
    }
}

void DataHandler::setDailyOrder(const QList<int> &order) {
    dOrder.order = order;
}

void DataHandler::deleteDailyTask(const int id, const QDate &deletionDate) {
    QMap<int, dailyTask> &map = dTasks.tasks;
    auto it = map.find(id);
    if (it != map.end()) {
        dailyTask& task = it.value();
        if (task.creationDate.daysTo(deletionDate) <= 0) {
            // The task has now been deleted on or before the day it was created
            // So delete the task completely

            dOrder.order.removeOne(task.id); // Delete from the order tracker
            for (dailyCompletion &dComp : dCompletion.dailyCompletions) {
                dComp.taskCompletion.remove(task.id); // This removes this task from each day's completeion list
            }
            map.erase(it); // And lastly delete it from the list of daily tasks

        } else {
            task.deletionDate = deletionDate;
        }
    } else {
        // This is where the error handling would go
        // This is not *supposed* to happen
        return;
    }

}

void DataHandler::setDailyTaskDays(const int id, const Days weekly) {
    dTasks.tasks[id].weekly = weekly;
}

const QList<int> &DataHandler::readDailyOrder() const{
    return dOrder.order;
}