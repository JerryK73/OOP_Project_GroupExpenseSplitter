#ifndef GROUP_H
#define GROUP_H

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include "Member.h"
#include "Expense.h"
#include "Settlement.h"

class Group
{
private:
    int id;
    std::string name;
    std::vector<Member> members;
    std::vector<Expense> expenses;
    std::vector<Settlement> settlements;

public:
    Group(const std::string &name, const std::vector<Member> &initialMembers);

    void addMember(const Member &member);
    void addExpense(const Expense &expense);
    void addSettlement(const Settlement &settlement);

    std::map<Member, double> getBalances() const;

    int getId() const;
    void setId(int id);
    std::string getName() const;
    std::vector<Member> getMembers() const;
    std::vector<Expense> getExpenses() const;
    std::vector<Settlement> getSettlements() const;

    bool hasMember(int memberId) const;
    Member getMemberById(int memberId) const;
};

#endif // GROUP_H
