#include "EqualSplitStrategy.h"
#include <cmath>
#include <stdexcept>

std::vector<Split> EqualSplitStrategy::calculateSplits(
    double amount,
    const std::vector<Member> &members) const
{

    if (members.empty())
    {
        throw std::invalid_argument("Cannot split an expense with zero members.");
    }

    std::vector<Split> splits;
    splits.reserve(members.size());

    long long totalCents = std::llround(amount * 100);
    long long baseCents = totalCents / static_cast<long long>(members.size());
    long long remainderCents = totalCents % static_cast<long long>(members.size());

    for (size_t i = 0; i < members.size(); ++i)
    {
        long long memberCents = baseCents + (static_cast<long long>(i) < remainderCents ? 1 : 0);
        splits.emplace_back(members[i], memberCents / 100.0);
    }

    return splits;
}
