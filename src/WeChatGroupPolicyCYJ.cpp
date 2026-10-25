#include "WeChatGroupPolicyCYJ.h"
std::string WeChatGroupPolicyCYJ::getName() const { return "WeChat"; }
bool WeChatGroupPolicyCYJ::canJoin(bool,bool recommended) const { return recommended; }
bool WeChatGroupPolicyCYJ::canKick(GroupRoleCYJ a,GroupRoleCYJ b) const { return a==GroupRoleCYJ::Owner && b!=GroupRoleCYJ::Owner; }
bool WeChatGroupPolicyCYJ::canCreateSubgroup(GroupRoleCYJ) const { return false; }
bool WeChatGroupPolicyCYJ::canManageAdmin(GroupRoleCYJ) const { return false; }
