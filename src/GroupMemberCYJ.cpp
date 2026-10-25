#include "GroupMemberCYJ.h"
#include <utility>
GroupMemberCYJ::GroupMemberCYJ(std::string id,GroupRoleCYJ r):accountId_(std::move(id)),role_(r) {}
const std::string& GroupMemberCYJ::getAccountId() const { return accountId_; }
GroupRoleCYJ GroupMemberCYJ::getRole() const { return role_; }
