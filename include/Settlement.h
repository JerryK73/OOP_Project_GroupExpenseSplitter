#ifndef SETTLEMENT_H
#define SETTLEMENT_H

#include <string>
#include "Member.h"

class Settlement
{
private:
    int id;
    Member from;
    Member to;
    double amount;
    std::string date;

public:
    Settlement(const Member &from, const Member &to, double amount, const std::string &date);

    int getId() const;
    void setId(int id);
    Member getFrom() const;
    Member getTo() const;
    double getAmount() const;
    std::string getDate() const;
};

#endif // SETTLEMENT_H
