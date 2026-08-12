#ifndef EXPENSE_H
#define EXPENSE_H

#include <string>
#include <vector>
#include <memory>
#include "Member.h"
#include "Split.h"
#include "SplitStrategy.h"

class Expense
{
private:
    int id;
    std::string description;
    double amount;
    Member paidBy;
    std::string date;
    std::vector<Member> participants;
    std::vector<Split> splits;
    std::shared_ptr<SplitStrategy> strategy;

public:
    Expense(const std::string &description,
            double amount,
            const Member &paidBy,
            const std::string &date,
            const std::vector<Member> &participants,
            std::shared_ptr<SplitStrategy> strategy);

    std::vector<Split> calculateSplits();

    void setSplits(const std::vector<Split> &loadedSplits);

    int getId() const;
    void setId(int id);
    std::string getDescription() const;
    double getAmount() const;
    Member getPaidBy() const;
    std::string getDate() const;
    std::vector<Member> getParticipants() const;
    std::vector<Split> getSplits() const;
};

#endif // EXPENSE_H
