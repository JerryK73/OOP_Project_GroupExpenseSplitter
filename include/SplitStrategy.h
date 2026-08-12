#ifndef SPLIT_STRATEGY_H
#define SPLIT_STRATEGY_H

#include <vector>
#include "Member.h"
#include "Split.h"

class SplitStrategy
{
public:
    virtual ~SplitStrategy() = default;
    virtual std::vector<Split> calculateSplits(
        double amount,
        const std::vector<Member> &members) const = 0;
};

#endif // SPLIT_STRATEGY_H
