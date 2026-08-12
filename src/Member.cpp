#include "Member.h"

Member::Member() : id(-1), name("") {}

Member::Member(int id, const std::string& name) : id(id), name(name) {}

int Member::getId() const {
    return id;
}

std::string Member::getName() const {
    return name;
}

bool Member::operator<(const Member& other) const {
    return id < other.id;
}

bool Member::operator==(const Member& other) const {
    return id == other.id;
}
