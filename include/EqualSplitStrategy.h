#ifndef EQUAL_SPLIT_STRATEGY_H
#define EQUAL_SPLIT_STRATEGY_H

#include "SplitStrategy.h"

class EqualSplitStrategy : public SplitStrategy
{
public:
    std::vector<Split> calculateSplits(
        double amount,
        const std::vector<Member> &members) const override;
};

#endif // EQUAL_SPLIT_STRATEGY_H
