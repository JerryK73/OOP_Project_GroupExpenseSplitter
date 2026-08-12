#ifndef EXPENSE_SPLITTER_CLI_H
#define EXPENSE_SPLITTER_CLI_H

#include <string>
#include <vector>
#include "Database.h"
#include "Group.h"

class ExpenseSplitterCLI
{
private:
    std::vector<Group> currentGroups;
    Database database;
    int nextMemberId;

    Group &selectGroup();
    Member promptForNewMember();
    int generateMemberId();

public:
    explicit ExpenseSplitterCLI(const std::string &dbPath);

    void run();
    void createGroupMenu();
    void addMemberMenu();
    void recordExpenseMenu();
    void viewBalancesMenu();
    void recordSettlementMenu();
    void viewHistoryMenu();
};

#endif // EXPENSE_SPLITTER_CLI_H
