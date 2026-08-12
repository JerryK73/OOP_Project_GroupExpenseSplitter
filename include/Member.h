#ifndef MEMBER_H
#define MEMBER_H

#include <string>

class Member
{
private:
    int id;
    std::string name;

public:
    Member();
    Member(int id, const std::string &name);

    int getId() const;
    std::string getName() const;

    bool operator<(const Member &other) const;
    bool operator==(const Member &other) const;
};

#endif // MEMBER_H
