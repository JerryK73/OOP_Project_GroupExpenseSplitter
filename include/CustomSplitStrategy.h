#ifndef CUSTOM_SPLIT_STRATEGY_H
#define CUSTOM_SPLIT_STRATEGY_H

#include <map>
#include "SplitStrategy.h"

class CustomSplitStrategy : public SplitStrategy
{
private:
    std::map<Member, double> customAmounts;

public:
    explicit CustomSplitStrategy(const std::map<Member, double> &customAmounts);

    std::vector<Split> calculateSplits(
        double amount,
        const std::vector<Member> &members) const override;
};

#endif // CUSTOM_SPLIT_STRATEGY_H
