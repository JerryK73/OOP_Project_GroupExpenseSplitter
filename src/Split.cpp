#include "Split.h"

Split::Split(const Member& member, double amountOwed)
    : member(member), amountOwed(amountOwed) {}

Member Split::getMember() const {
    return member;
}

double Split::getAmountOwed() const {
    return amountOwed;
}
