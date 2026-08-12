#include "Group.h"

Group::Group(const std::string &name, const std::vector<Member> &initialMembers)
    : id(-1), name(name), members(initialMembers)
{
    if (members.size() < 2)
    {
        throw std::invalid_argument("A group needs at least two initial members.");
    }
}

void Group::addMember(const Member &member)
{
    if (hasMember(member.getId()))
    {
        throw std::invalid_argument("Member already exists in this group.");
    }
    members.push_back(member);
}

void Group::addExpense(const Expense &expense)
{
    expenses.push_back(expense);
}

void Group::addSettlement(const Settlement &settlement)
{
    settlements.push_back(settlement);
}

std::map<Member, double> Group::getBalances() const
{
    std::map<Member, double> balances;

    for (const auto &member : members)
    {
        balances[member] = 0.0;
    }

    for (const auto &expense : expenses)
    {
        balances[expense.getPaidBy()] += expense.getAmount();
        for (const auto &split : expense.getSplits())
        {
            balances[split.getMember()] -= split.getAmountOwed();
        }
    }

    for (const auto &settlement : settlements)
    {
        balances[settlement.getFrom()] += settlement.getAmount();
        balances[settlement.getTo()] -= settlement.getAmount();
    }

    return balances;
}

int Group::getId() const { return id; }
void Group::setId(int newId) { id = newId; }
std::string Group::getName() const { return name; }
std::vector<Member> Group::getMembers() const { return members; }
std::vector<Expense> Group::getExpenses() const { return expenses; }
std::vector<Settlement> Group::getSettlements() const { return settlements; }

bool Group::hasMember(int memberId) const
{
    for (const auto &member : members)
    {
        if (member.getId() == memberId)
        {
            return true;
        }
    }
    return false;
}

Member Group::getMemberById(int memberId) const
{
    for (const auto &member : members)
    {
        if (member.getId() == memberId)
        {
            return member;
        }
    }
    throw std::invalid_argument("Member not found in group.");
}
