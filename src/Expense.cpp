#include "Expense.h"

Expense::Expense(const std::string& description,
                  double amount,
                  const Member& paidBy,
                  const std::string& date,
                  const std::vector<Member>& participants,
                  std::shared_ptr<SplitStrategy> strategy)
    : id(-1),
      description(description),
      amount(amount),
      paidBy(paidBy),
      date(date),
      participants(participants),
      strategy(strategy) {}

std::vector<Split> Expense::calculateSplits() {
    splits = strategy->calculateSplits(amount, participants);
    return splits;
}

void Expense::setSplits(const std::vector<Split>& loadedSplits) {
    splits = loadedSplits;
}

int Expense::getId() const { return id; }
void Expense::setId(int newId) { id = newId; }
std::string Expense::getDescription() const { return description; }
double Expense::getAmount() const { return amount; }
Member Expense::getPaidBy() const { return paidBy; }
std::string Expense::getDate() const { return date; }
std::vector<Member> Expense::getParticipants() const { return participants; }
std::vector<Split> Expense::getSplits() const { return splits; }
