#include "ExpenseSplitterCLI.h"
#include "EqualSplitStrategy.h"
#include "CustomSplitStrategy.h"
#include <iostream>
#include <sstream>
#include <limits>
#include <ctime>
#include <algorithm>

namespace
{

    std::string readLine(const std::string &prompt)
    {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        return line;
    }

    double readPositiveDouble(const std::string &prompt)
    {
        while (true)
        {
            std::string line = readLine(prompt);
            try
            {
                double value = std::stod(line);
                if (value <= 0)
                {
                    std::cout << "Amount must be greater than zero. Try again.\n";
                    continue;
                }
                return value;
            }
            catch (...)
            {
                std::cout << "Invalid number. Try again.\n";
            }
        }
    }

    int readInt(const std::string &prompt)
    {
        while (true)
        {
            std::string line = readLine(prompt);
            try
            {
                return std::stoi(line);
            }
            catch (...)
            {
                std::cout << "Invalid number. Try again.\n";
            }
        }
    }

    std::string todayDate()
    {
        std::time_t t = std::time(nullptr);
        std::tm *tm = std::localtime(&t);
        char buffer[11];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", tm);
        return std::string(buffer);
    }

} // namespace

ExpenseSplitterCLI::ExpenseSplitterCLI(const std::string &dbPath)
    : database(dbPath), nextMemberId(1)
{
    currentGroups = database.loadGroups();

    for (const auto &group : currentGroups)
    {
        for (const auto &member : group.getMembers())
        {
            nextMemberId = std::max(nextMemberId, member.getId() + 1);
        }
    }
}

int ExpenseSplitterCLI::generateMemberId()
{
    return nextMemberId++;
}

Group &ExpenseSplitterCLI::selectGroup()
{
    if (currentGroups.empty())
    {
        throw std::runtime_error("No groups exist yet. Create one first.");
    }
    std::cout << "\nGroups:\n";
    for (size_t i = 0; i < currentGroups.size(); ++i)
    {
        std::cout << "  " << i + 1 << ". " << currentGroups[i].getName() << "\n";
    }
    int choice = readInt("Select a group by number: ");
    if (choice < 1 || static_cast<size_t>(choice) > currentGroups.size())
    {
        throw std::runtime_error("Invalid group selection.");
    }
    return currentGroups[choice - 1];
}

Member ExpenseSplitterCLI::promptForNewMember()
{
    std::string name = readLine("  Member name: ");
    return Member(generateMemberId(), name);
}

void ExpenseSplitterCLI::createGroupMenu()
{
    std::cout << "\n-- Create Group --\n";
    std::string name = readLine("Group name: ");

    std::vector<Member> members;
    std::cout << "Enter at least two initial members (blank line to finish once you have 2+):\n";
    while (true)
    {
        std::string line = readLine("  Member name (blank to finish): ");
        if (line.empty())
        {
            if (members.size() >= 2)
            {
                break;
            }
            std::cout << "A group needs at least two members before you can finish.\n";
            continue;
        }
        members.emplace_back(generateMemberId(), line);
    }

    try
    {
        Group group(name, members);
        database.saveGroup(group);
        currentGroups.push_back(group);
        std::cout << "Group \"" << name << "\" created with " << members.size() << " members.\n";
    }
    catch (const std::exception &e)
    {
        std::cout << "Error creating group: " << e.what() << "\n";
    }
}

void ExpenseSplitterCLI::addMemberMenu()
{
    std::cout << "\n-- Add Member --\n";
    try
    {
        Group &group = selectGroup();
        Member member = promptForNewMember();
        group.addMember(member);
        database.saveGroup(group);
        std::cout << "Added " << member.getName() << " to " << group.getName() << ".\n";
    }
    catch (const std::exception &e)
    {
        std::cout << "Error adding member: " << e.what() << "\n";
    }
}

void ExpenseSplitterCLI::recordExpenseMenu()
{
    std::cout << "\n-- Record Expense --\n";
    try
    {
        Group &group = selectGroup();

        std::string description = readLine("Description: ");
        double amount = readPositiveDouble("Amount: ");

        std::cout << "Members in this group:\n";
        const auto &groupMembers = group.getMembers();
        for (size_t i = 0; i < groupMembers.size(); ++i)
        {
            std::cout << "  " << i + 1 << ". " << groupMembers[i].getName() << "\n";
        }
        int paidByChoice = readInt("Who paid? (number): ");
        if (paidByChoice < 1 || static_cast<size_t>(paidByChoice) > groupMembers.size())
        {
            throw std::runtime_error("Invalid selection for who paid.");
        }
        Member paidBy = groupMembers[paidByChoice - 1];

        std::cout << "Enter member numbers this expense applies to, separated by spaces "
                     "(e.g. \"1 2 3\"), or blank for everyone:\n";
        std::string participantLine = readLine("  Members: ");
        std::vector<Member> participants;
        if (participantLine.empty())
        {
            participants = groupMembers;
        }
        else
        {
            std::istringstream iss(participantLine);
            int idx;
            while (iss >> idx)
            {
                if (idx < 1 || static_cast<size_t>(idx) > groupMembers.size())
                {
                    throw std::runtime_error("Invalid member number in participant list.");
                }
                participants.push_back(groupMembers[idx - 1]);
            }
        }
        if (participants.empty())
        {
            throw std::runtime_error("An expense must apply to at least one member.");
        }

        std::cout << "Split method:\n  1. Split Equally\n  2. Split by Custom Amount\n";
        int splitChoice = readInt("Choice: ");

        std::shared_ptr<SplitStrategy> strategy;
        if (splitChoice == 1)
        {
            strategy = std::make_shared<EqualSplitStrategy>();
        }
        else if (splitChoice == 2)
        {
            std::map<Member, double> customAmounts;
            double sum = 0.0;
            for (const auto &member : participants)
            {
                double owed = readPositiveDouble("  Amount owed by " + member.getName() + ": ");
                customAmounts[member] = owed;
                sum += owed;
            }
            if (std::abs(sum - amount) > 0.01)
            {
                throw std::runtime_error(
                    "Custom amounts (" + std::to_string(sum) +
                    ") must sum to the total expense amount (" + std::to_string(amount) + ").");
            }
            strategy = std::make_shared<CustomSplitStrategy>(customAmounts);
        }
        else
        {
            throw std::runtime_error("Invalid split method selection.");
        }

        Expense expense(description, amount, paidBy, todayDate(), participants, strategy);
        expense.calculateSplits();

        group.addExpense(expense);
        database.saveExpense(expense, group.getId());

        std::cout << "Expense recorded. Splits:\n";
        for (const auto &split : expense.getSplits())
        {
            std::cout << "  " << split.getMember().getName() << " owes $" << split.getAmountOwed() << "\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Error recording expense: " << e.what() << "\n";
    }
}

void ExpenseSplitterCLI::viewBalancesMenu()
{
    std::cout << "\n-- View Balances --\n";
    try
    {
        Group &group = selectGroup();
        auto balances = group.getBalances();
        if (balances.empty())
        {
            std::cout << "No members in this group.\n";
            return;
        }
        for (const auto &entry : balances)
        {
            const Member &member = entry.first;
            double balance = entry.second;
            if (balance > 0.005)
            {
                std::cout << "  " << member.getName() << " is owed $" << balance << "\n";
            }
            else if (balance < -0.005)
            {
                std::cout << "  " << member.getName() << " owes $" << -balance << "\n";
            }
            else
            {
                std::cout << "  " << member.getName() << " is settled up\n";
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Error viewing balances: " << e.what() << "\n";
    }
}

void ExpenseSplitterCLI::recordSettlementMenu()
{
    std::cout << "\n-- Record Settlement --\n";
    try
    {
        Group &group = selectGroup();
        const auto &groupMembers = group.getMembers();
        for (size_t i = 0; i < groupMembers.size(); ++i)
        {
            std::cout << "  " << i + 1 << ". " << groupMembers[i].getName() << "\n";
        }
        int fromChoice = readInt("Who is paying? (number): ");
        int toChoice = readInt("Who is receiving? (number): ");
        if (fromChoice < 1 || static_cast<size_t>(fromChoice) > groupMembers.size() ||
            toChoice < 1 || static_cast<size_t>(toChoice) > groupMembers.size())
        {
            throw std::runtime_error("Invalid member selection.");
        }
        if (fromChoice == toChoice)
        {
            throw std::runtime_error("A member cannot settle with themselves.");
        }
        double amount = readPositiveDouble("Amount: ");

        Settlement settlement(groupMembers[fromChoice - 1], groupMembers[toChoice - 1], amount, todayDate());
        group.addSettlement(settlement);
        database.saveSettlement(settlement, group.getId());
        std::cout << "Settlement recorded.\n";
    }
    catch (const std::exception &e)
    {
        std::cout << "Error recording settlement: " << e.what() << "\n";
    }
}

void ExpenseSplitterCLI::viewHistoryMenu()
{
    std::cout << "\n-- View Expense History --\n";
    try
    {
        Group &group = selectGroup();
        std::cout << "Expenses:\n";
        for (const auto &expense : group.getExpenses())
        {
            std::cout << "  [" << expense.getDate() << "] " << expense.getDescription()
                      << " - $" << expense.getAmount() << " paid by " << expense.getPaidBy().getName() << "\n";
        }
        std::cout << "Settlements:\n";
        for (const auto &settlement : group.getSettlements())
        {
            std::cout << "  [" << settlement.getDate() << "] " << settlement.getFrom().getName()
                      << " paid " << settlement.getTo().getName() << " $" << settlement.getAmount() << "\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Error viewing history: " << e.what() << "\n";
    }
}

void ExpenseSplitterCLI::run()
{
    std::cout << "=== Expense Splitter ===\n";
    while (true)
    {
        std::cout << "\n1. Create Group\n"
                     "2. Add Member\n"
                     "3. Record Expense\n"
                     "4. View Balances\n"
                     "5. Record Settlement\n"
                     "6. View Expense History\n"
                     "7. Exit\n";
        int choice = readInt("Choose an option: ");
        switch (choice)
        {
        case 1:
            createGroupMenu();
            break;
        case 2:
            addMemberMenu();
            break;
        case 3:
            recordExpenseMenu();
            break;
        case 4:
            viewBalancesMenu();
            break;
        case 5:
            recordSettlementMenu();
            break;
        case 6:
            viewHistoryMenu();
            break;
        case 7:
            std::cout << "Goodbye!\n";
            return;
        default:
            std::cout << "Invalid option.\n";
        }
    }
}
