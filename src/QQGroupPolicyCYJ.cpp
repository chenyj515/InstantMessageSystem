#include "QQGroupPolicyCYJ.h"
std::string QQGroupPolicyCYJ::getName() const { return "QQ"; }
// QQ: 可申请加入（审批），也可由群内好友推荐加入；微信才是“只能推荐”。
bool QQGroupPolicyCYJ::canJoin(bool approved,bool recommended) const { return approved || recommended; }
bool QQGroupPolicyCYJ::canKick(GroupRoleCYJ a,GroupRoleCYJ b) const {
    return b!=GroupRoleCYJ::Owner && (a==GroupRoleCYJ::Owner || (a==GroupRoleCYJ::Admin && b==GroupRoleCYJ::Member));
}
bool QQGroupPolicyCYJ::canCreateSubgroup(GroupRoleCYJ a) const { return a==GroupRoleCYJ::Owner || a==GroupRoleCYJ::Admin; }
bool QQGroupPolicyCYJ::canManageAdmin(GroupRoleCYJ a) const { return a==GroupRoleCYJ::Owner; }
