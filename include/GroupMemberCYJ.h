#pragma once
#include "TypesCYJ.h"
class GroupMemberCYJ {
public:
    GroupMemberCYJ(std::string account, GroupRoleCYJ role = GroupRoleCYJ::Member);
    const std::string& getAccountId() const;
    GroupRoleCYJ getRole() const;
private:
    friend class GroupCYJ;
    friend class GroupManagerCYJ;
    std::string accountId_;
    GroupRoleCYJ role_;
};
