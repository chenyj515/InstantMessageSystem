#pragma once
#include "GroupPolicyCYJ.h"
class WeChatGroupPolicyCYJ final : public GroupPolicyCYJ {
public:
    std::string getName() const override;
    bool canJoin(bool approved, bool recommended) const override;
    bool canKick(GroupRoleCYJ actor, GroupRoleCYJ target) const override;
    bool canCreateSubgroup(GroupRoleCYJ actor) const override;
    bool canManageAdmin(GroupRoleCYJ actor) const override;
};
