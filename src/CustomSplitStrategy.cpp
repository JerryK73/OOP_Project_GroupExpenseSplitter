#include "CustomSplitStrategy.h"
#include <stdexcept>
#include <cmath>

CustomSplitStrategy::CustomSplitStrategy(const std::map<Member, double> &customAmounts)
    : customAmounts(customAmounts) {}

std::vector<Split> CustomSplitStrategy::calculateSplits(
    double amount,
    const std::vector<Member> &members) const
{

    double sum = 0.0;
    for (const auto &entry : customAmounts)
    {
        sum += entry.second;
    }

    if (std::abs(sum - amount) > 0.01)
    {
        throw std::invalid_argument(
            "Custom split amounts must sum to the total expense amount.");
    }

    std::vector<Split> splits;
    splits.reserve(members.size());
    for (const auto &member : members)
    {
        auto it = customAmounts.find(member);
        if (it == customAmounts.end())
        {
            throw std::invalid_argument(
                "Missing custom amount for member: " + member.getName());
        }
        splits.emplace_back(member, it->second);
    }

    return splits;
}
