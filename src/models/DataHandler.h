#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include "JsonHandlers.h"
#include <QString>


class DataHandler {
    public:
        // Meyers Singleton implementation
        static DataHandler& instance() {
            static DataHandler _instance; //Created once the first time it's called, then persists until program termination
            return _instance;
        }

        void loadAll();
        void saveAll();

        const QMap<int, todoTask> &readAllTodoTasks() const;
        const todoTask readTodoTaskById(const int id) const;
        const QList<int> &readTodoOrder() const;
        int createTodoTask(const QString &name); // returns the id of the newly created task
        void deleteTodoTask(const int id);
        void setTodoTaskName(const int id, const QString &name);
        void setTodoTaskCompletion(const int id, const int completion);
        void setTodoTaskDate(const int id, const QDate &dueDate);
        void setTodoOrder(const QList<int> &order);
        
        const QMap<int, dailyTask> &readAllDailyTasks() const;
        const dailyTask readDailyTaskById(const int id) const;
        const QList<int> &readDailyOrder() const;
        const QMap<int, int> &readDailyCompletionByDay(const QDate &date);
        int readCompletionByDateId(const QDate &date, const int) const;
        int createDailyTask(const QString &name, const QDate &creationDate); // returns the id of the newly created task
        void deleteDailyTask(const int id, const QDate &deletionDate);
        void setDailyTaskName(const int id, const QString &name);
        void setDailyTaskDays(const int id, const Days weekly);
        void setDailyOrder(const QList<int> &order);
        void setDailyTaskCompletion(const int id, const QDate &date, const int completion);

    private:
        const QString tTasksFilename = "todoTasks.json";
        const QString tOrderFilename = "todoOrder.json";
        const QString dTasksFilename = "dailyTasks.json";
        const QString dOrderFilename = "dailyOrder.json";
        const QString dCompletionFilename = "dailyCompletion.json";

        todoTaskList tTasks;
        todoOrder tOrder;
        dailyTaskList dTasks;
        dailyOrder dOrder;
        dailyCompletionMap dCompletion;

        DataHandler() {};
        QString getStorageFilePath(const QString &fileName) const;
        
        template <typename T>
        bool loadFromJsonFile(T &target, const QString &filename) {
            QFile file(getStorageFilePath(filename));

            if (!file.open(QIODevice::ReadOnly)) {
                return false; // Better error handling would be nice
            }

            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            
            if (doc.isNull() || !doc.isObject()) {
                return false; // Basic validation, better error handling would be nice
            }

            target.fromJson(doc.object());
            return true;
        }

        template <typename T>
        bool saveToJsonFile(const T &source, const QString &filename) {
            QFile file(getStorageFilePath(filename));

            if (!file.open(QIODevice::WriteOnly)) {
                return false;
            }

            QJsonDocument doc(source.toJson());
            
            // Indented makes it human-readable, although takes up more space
            file.write(doc.toJson(QJsonDocument::Indented));
            return true;
        }
};

#endif