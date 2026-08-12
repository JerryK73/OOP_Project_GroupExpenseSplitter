#ifndef SPLIT_H
#define SPLIT_H

#include "Member.h"

class Split
{
private:
    Member member;
    double amountOwed;

public:
    Split(const Member &member, double amountOwed);

    Member getMember() const;
    double getAmountOwed() const;
};

#endif // SPLIT_H
