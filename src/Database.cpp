#include "Database.h"
#include <stdexcept>
#include <map>

Database::Database(const std::string &dbPath) : dbPath(dbPath), db(nullptr)
{
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK)
    {
        std::string error = db ? sqlite3_errmsg(db) : "unknown error";
        throw std::runtime_error("Failed to open database: " + error);
    }
    execOrThrow("PRAGMA foreign_keys = ON;");
    initSchema();
}

Database::~Database()
{
    if (db)
    {
        sqlite3_close(db);
    }
}

void Database::execOrThrow(const std::string &sql)
{
    char *errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::string error = errMsg ? errMsg : "unknown error";
        sqlite3_free(errMsg);
        throw std::runtime_error("SQL error: " + error);
    }
}

void Database::initSchema()
{
    execOrThrow(
        "CREATE TABLE IF NOT EXISTS groups ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL"
        ");");

    execOrThrow(
        "CREATE TABLE IF NOT EXISTS members ("
        "  id INTEGER PRIMARY KEY,"
        "  group_id INTEGER NOT NULL,"
        "  name TEXT NOT NULL,"
        "  FOREIGN KEY(group_id) REFERENCES groups(id)"
        ");");

    execOrThrow(
        "CREATE TABLE IF NOT EXISTS expenses ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  group_id INTEGER NOT NULL,"
        "  description TEXT NOT NULL,"
        "  amount REAL NOT NULL,"
        "  paid_by_member_id INTEGER NOT NULL,"
        "  date TEXT NOT NULL,"
        "  FOREIGN KEY(group_id) REFERENCES groups(id),"
        "  FOREIGN KEY(paid_by_member_id) REFERENCES members(id)"
        ");");

    execOrThrow(
        "CREATE TABLE IF NOT EXISTS expense_participants ("
        "  expense_id INTEGER NOT NULL,"
        "  member_id INTEGER NOT NULL,"
        "  FOREIGN KEY(expense_id) REFERENCES expenses(id),"
        "  FOREIGN KEY(member_id) REFERENCES members(id)"
        ");");

    execOrThrow(
        "CREATE TABLE IF NOT EXISTS splits ("
        "  expense_id INTEGER NOT NULL,"
        "  member_id INTEGER NOT NULL,"
        "  amount_owed REAL NOT NULL,"
        "  FOREIGN KEY(expense_id) REFERENCES expenses(id),"
        "  FOREIGN KEY(member_id) REFERENCES members(id)"
        ");");

    execOrThrow(
        "CREATE TABLE IF NOT EXISTS settlements ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  group_id INTEGER NOT NULL,"
        "  from_member_id INTEGER NOT NULL,"
        "  to_member_id INTEGER NOT NULL,"
        "  amount REAL NOT NULL,"
        "  date TEXT NOT NULL,"
        "  FOREIGN KEY(group_id) REFERENCES groups(id),"
        "  FOREIGN KEY(from_member_id) REFERENCES members(id),"
        "  FOREIGN KEY(to_member_id) REFERENCES members(id)"
        ");");
}

void Database::saveGroup(Group &group)
{
    if (group.getId() == -1)
    {
        sqlite3_stmt *stmt = nullptr;
        const char *sql = "INSERT INTO groups(name) VALUES(?);";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Failed to prepare insert group statement.");
        }
        sqlite3_bind_text(stmt, 1, group.getName().c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to insert group.");
        }
        sqlite3_finalize(stmt);
        group.setId(static_cast<int>(sqlite3_last_insert_rowid(db)));
    }
    else
    {
        sqlite3_stmt *stmt = nullptr;
        const char *sql = "UPDATE groups SET name = ? WHERE id = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Failed to prepare update group statement.");
        }
        sqlite3_bind_text(stmt, 1, group.getName().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, group.getId());
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to update group.");
        }
        sqlite3_finalize(stmt);
    }

    const char *memberSql =
        "INSERT INTO members(id, group_id, name) VALUES(?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET group_id = excluded.group_id, name = excluded.name;";
    sqlite3_stmt *memberStmt = nullptr;
    if (sqlite3_prepare_v2(db, memberSql, -1, &memberStmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare upsert member statement.");
    }
    for (const auto &member : group.getMembers())
    {
        sqlite3_bind_int(memberStmt, 1, member.getId());
        sqlite3_bind_int(memberStmt, 2, group.getId());
        sqlite3_bind_text(memberStmt, 3, member.getName().c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(memberStmt) != SQLITE_DONE)
        {
            sqlite3_finalize(memberStmt);
            throw std::runtime_error("Failed to upsert member.");
        }
        sqlite3_reset(memberStmt);
    }
    sqlite3_finalize(memberStmt);
}

void Database::saveExpense(Expense &expense, int groupId)
{
    sqlite3_stmt *stmt = nullptr;
    const char *sql =
        "INSERT INTO expenses(group_id, description, amount, paid_by_member_id, date) "
        "VALUES(?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare insert expense statement.");
    }
    sqlite3_bind_int(stmt, 1, groupId);
    sqlite3_bind_text(stmt, 2, expense.getDescription().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, expense.getAmount());
    sqlite3_bind_int(stmt, 4, expense.getPaidBy().getId());
    sqlite3_bind_text(stmt, 5, expense.getDate().c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to insert expense.");
    }
    sqlite3_finalize(stmt);
    int expenseId = static_cast<int>(sqlite3_last_insert_rowid(db));
    expense.setId(expenseId);

    sqlite3_stmt *participantStmt = nullptr;
    const char *participantSql = "INSERT INTO expense_participants(expense_id, member_id) VALUES(?, ?);";
    if (sqlite3_prepare_v2(db, participantSql, -1, &participantStmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare insert participant statement.");
    }
    for (const auto &member : expense.getParticipants())
    {
        sqlite3_bind_int(participantStmt, 1, expenseId);
        sqlite3_bind_int(participantStmt, 2, member.getId());
        if (sqlite3_step(participantStmt) != SQLITE_DONE)
        {
            sqlite3_finalize(participantStmt);
            throw std::runtime_error("Failed to insert expense participant.");
        }
        sqlite3_reset(participantStmt);
    }
    sqlite3_finalize(participantStmt);

    sqlite3_stmt *splitStmt = nullptr;
    const char *splitSql = "INSERT INTO splits(expense_id, member_id, amount_owed) VALUES(?, ?, ?);";
    if (sqlite3_prepare_v2(db, splitSql, -1, &splitStmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare insert split statement.");
    }
    for (const auto &split : expense.getSplits())
    {
        sqlite3_bind_int(splitStmt, 1, expenseId);
        sqlite3_bind_int(splitStmt, 2, split.getMember().getId());
        sqlite3_bind_double(splitStmt, 3, split.getAmountOwed());
        if (sqlite3_step(splitStmt) != SQLITE_DONE)
        {
            sqlite3_finalize(splitStmt);
            throw std::runtime_error("Failed to insert split.");
        }
        sqlite3_reset(splitStmt);
    }
    sqlite3_finalize(splitStmt);
}

void Database::saveSettlement(Settlement &settlement, int groupId)
{
    sqlite3_stmt *stmt = nullptr;
    const char *sql =
        "INSERT INTO settlements(group_id, from_member_id, to_member_id, amount, date) "
        "VALUES(?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare insert settlement statement.");
    }
    sqlite3_bind_int(stmt, 1, groupId);
    sqlite3_bind_int(stmt, 2, settlement.getFrom().getId());
    sqlite3_bind_int(stmt, 3, settlement.getTo().getId());
    sqlite3_bind_double(stmt, 4, settlement.getAmount());
    sqlite3_bind_text(stmt, 5, settlement.getDate().c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to insert settlement.");
    }
    sqlite3_finalize(stmt);
    settlement.setId(static_cast<int>(sqlite3_last_insert_rowid(db)));
}

std::vector<Expense> Database::getExpenseHistory(int groupId)
{
    std::map<int, Member> membersById;
    {
        sqlite3_stmt *stmt = nullptr;
        const char *sql = "SELECT id, name FROM members WHERE group_id = ?;";
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, groupId);
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            membersById[id] = Member(id, name);
        }
        sqlite3_finalize(stmt);
    }

    std::vector<Expense> expenses;
    sqlite3_stmt *expenseStmt = nullptr;
    const char *expenseSql =
        "SELECT id, description, amount, paid_by_member_id, date "
        "FROM expenses WHERE group_id = ? ORDER BY date;";
    sqlite3_prepare_v2(db, expenseSql, -1, &expenseStmt, nullptr);
    sqlite3_bind_int(expenseStmt, 1, groupId);

    while (sqlite3_step(expenseStmt) == SQLITE_ROW)
    {
        int expenseId = sqlite3_column_int(expenseStmt, 0);
        std::string description = reinterpret_cast<const char *>(sqlite3_column_text(expenseStmt, 1));
        double amount = sqlite3_column_double(expenseStmt, 2);
        int paidById = sqlite3_column_int(expenseStmt, 3);
        std::string date = reinterpret_cast<const char *>(sqlite3_column_text(expenseStmt, 4));

        std::vector<Member> participants;
        sqlite3_stmt *pStmt = nullptr;
        const char *pSql = "SELECT member_id FROM expense_participants WHERE expense_id = ?;";
        sqlite3_prepare_v2(db, pSql, -1, &pStmt, nullptr);
        sqlite3_bind_int(pStmt, 1, expenseId);
        while (sqlite3_step(pStmt) == SQLITE_ROW)
        {
            int memberId = sqlite3_column_int(pStmt, 0);
            participants.push_back(membersById[memberId]);
        }
        sqlite3_finalize(pStmt);

        std::vector<Split> splits;
        sqlite3_stmt *sStmt = nullptr;
        const char *sSql = "SELECT member_id, amount_owed FROM splits WHERE expense_id = ?;";
        sqlite3_prepare_v2(db, sSql, -1, &sStmt, nullptr);
        sqlite3_bind_int(sStmt, 1, expenseId);
        while (sqlite3_step(sStmt) == SQLITE_ROW)
        {
            int memberId = sqlite3_column_int(sStmt, 0);
            double amountOwed = sqlite3_column_double(sStmt, 1);
            splits.emplace_back(membersById[memberId], amountOwed);
        }
        sqlite3_finalize(sStmt);

        Expense expense(description, amount, membersById[paidById], date, participants, nullptr);
        expense.setId(expenseId);
        expense.setSplits(splits);
        expenses.push_back(expense);
    }
    sqlite3_finalize(expenseStmt);

    return expenses;
}

std::vector<Group> Database::loadGroups()
{
    std::vector<Group> groups;

    sqlite3_stmt *groupStmt = nullptr;
    const char *groupSql = "SELECT id, name FROM groups;";
    sqlite3_prepare_v2(db, groupSql, -1, &groupStmt, nullptr);

    while (sqlite3_step(groupStmt) == SQLITE_ROW)
    {
        int groupId = sqlite3_column_int(groupStmt, 0);
        std::string name = reinterpret_cast<const char *>(sqlite3_column_text(groupStmt, 1));

        std::vector<Member> members;
        sqlite3_stmt *memberStmt = nullptr;
        const char *memberSql = "SELECT id, name FROM members WHERE group_id = ?;";
        sqlite3_prepare_v2(db, memberSql, -1, &memberStmt, nullptr);
        sqlite3_bind_int(memberStmt, 1, groupId);
        while (sqlite3_step(memberStmt) == SQLITE_ROW)
        {
            int memberId = sqlite3_column_int(memberStmt, 0);
            std::string memberName = reinterpret_cast<const char *>(sqlite3_column_text(memberStmt, 1));
            members.emplace_back(memberId, memberName);
        }
        sqlite3_finalize(memberStmt);

        if (members.size() < 2)
        {
            // Skip malformed/incomplete groups rather than crashing the app.
            continue;
        }

        Group group(name, members);
        group.setId(groupId);

        for (const auto &expense : getExpenseHistory(groupId))
        {
            group.addExpense(expense);
        }

        sqlite3_stmt *settlementStmt = nullptr;
        const char *settlementSql =
            "SELECT from_member_id, to_member_id, amount, date FROM settlements WHERE group_id = ? ORDER BY date;";
        sqlite3_prepare_v2(db, settlementSql, -1, &settlementStmt, nullptr);
        sqlite3_bind_int(settlementStmt, 1, groupId);
        while (sqlite3_step(settlementStmt) == SQLITE_ROW)
        {
            int fromId = sqlite3_column_int(settlementStmt, 0);
            int toId = sqlite3_column_int(settlementStmt, 1);
            double amount = sqlite3_column_double(settlementStmt, 2);
            std::string date = reinterpret_cast<const char *>(sqlite3_column_text(settlementStmt, 3));
            group.addSettlement(Settlement(group.getMemberById(fromId), group.getMemberById(toId), amount, date));
        }
        sqlite3_finalize(settlementStmt);

        groups.push_back(group);
    }
    sqlite3_finalize(groupStmt);

    return groups;
}
