#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <sqlite3.h>
#include "Group.h"
#include "Expense.h"
#include "Settlement.h"

class Database
{
private:
    std::string dbPath;
    sqlite3 *db;

    void initSchema();
    void execOrThrow(const std::string &sql);

public:
    explicit Database(const std::string &dbPath);
    ~Database();

    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    void saveGroup(Group &group);
    std::vector<Group> loadGroups();
    void saveExpense(Expense &expense, int groupId);
    void saveSettlement(Settlement &settlement, int groupId);
    std::vector<Expense> getExpenseHistory(int groupId);
};

#endif // DATABASE_H
