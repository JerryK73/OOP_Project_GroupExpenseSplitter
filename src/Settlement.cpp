#include "Settlement.h"

Settlement::Settlement(const Member& from, const Member& to, double amount, const std::string& date)
    : id(-1), from(from), to(to), amount(amount), date(date) {}

int Settlement::getId() const { return id; }
void Settlement::setId(int newId) { id = newId; }
Member Settlement::getFrom() const { return from; }
Member Settlement::getTo() const { return to; }
double Settlement::getAmount() const { return amount; }
std::string Settlement::getDate() const { return date; }
