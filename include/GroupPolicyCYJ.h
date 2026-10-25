#pragma once
#include "TypesCYJ.h"
#include <memory>
class GroupPolicyCYJ {
public:
    virtual ~GroupPolicyCYJ();
    virtual std::string getName() const = 0;
    virtual bool canJoin(bool approved, bool recommended) const = 0;
    virtual bool canKick(GroupRoleCYJ actor, GroupRoleCYJ target) const = 0;
    virtual bool canCreateSubgroup(GroupRoleCYJ actor) const = 0;
    virtual bool canManageAdmin(GroupRoleCYJ actor) const = 0;
    static std::unique_ptr<GroupPolicyCYJ> create(const std::string& name);
};
