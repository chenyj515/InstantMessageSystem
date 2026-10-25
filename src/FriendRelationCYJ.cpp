#include "FriendRelationCYJ.h"
#include "TypesCYJ.h"
#include <utility>
FriendRelationCYJ::FriendRelationCYJ(std::string a,std::string b,std::string r):accountId_(std::move(a)),friendAccountId_(std::move(b)) { updateRemark(r); }
const std::string& FriendRelationCYJ::getAccountId() const { return accountId_; }
const std::string& FriendRelationCYJ::getFriendAccountId() const { return friendAccountId_; }
const std::string& FriendRelationCYJ::getRemark() const { return remark_; }
void FriendRelationCYJ::updateRemark(const std::string& r) { validateTextCYJ(r,"Remark",true); remark_=r; }
