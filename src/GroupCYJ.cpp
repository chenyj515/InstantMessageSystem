#include "GroupCYJ.h"
#include <utility>
GroupCYJ::GroupCYJ(std::string id,std::string name,std::string owner,ServiceTypeCYJ service,std::unique_ptr<GroupPolicyCYJ> policy,std::string parent)
 :groupId_(std::move(id)),name_(std::move(name)),ownerAccountId_(std::move(owner)),parentGroupId_(std::move(parent)),service_(service),policy_(std::move(policy)) {
    validateIdCYJ(groupId_); validateTextCYJ(name_,"Group name"); requireCYJ(policy_!=nullptr,"Missing policy");
    requireCYJ(service_!=ServiceTypeCYJ::Weibo,"Weibo groups are not supported");
    members_.emplace(ownerAccountId_,GroupMemberCYJ(ownerAccountId_,GroupRoleCYJ::Owner));
}
const std::string& GroupCYJ::getGroupId() const { return groupId_; }
const std::string& GroupCYJ::getName() const { return name_; }
const std::string& GroupCYJ::getOwnerAccountId() const { return ownerAccountId_; }
const std::string& GroupCYJ::getParentGroupId() const { return parentGroupId_; }
ServiceTypeCYJ GroupCYJ::getServiceType() const { return service_; }
const GroupPolicyCYJ& GroupCYJ::getPolicy() const { return *policy_; }
const std::map<std::string,GroupMemberCYJ>& GroupCYJ::getMembers() const { return members_; }
const std::set<std::string>& GroupCYJ::getPendingRequests() const { return pending_; }
bool GroupCYJ::hasMember(const std::string& a) const { return members_.count(a)!=0; }
GroupRoleCYJ GroupCYJ::roleOf(const std::string& a) const { requireCYJ(hasMember(a),"Account is not a group member"); return members_.at(a).getRole(); }
void GroupCYJ::addMember(const std::string& a) { requireCYJ(!hasMember(a),"Already in group"); members_.emplace(a,GroupMemberCYJ(a)); pending_.erase(a); }
void GroupCYJ::removeMember(const std::string& a) { requireCYJ(a!=ownerAccountId_,"Owner must dissolve the group instead"); requireCYJ(hasMember(a),"Member not found"); members_.erase(a); }
void GroupCYJ::setPolicy(std::unique_ptr<GroupPolicyCYJ> p) {
    requireCYJ(p!=nullptr,"Missing policy");
    if(!p->canManageAdmin(GroupRoleCYJ::Owner)) {
        for(auto& [id,m]:members_) { (void)id; if(m.role_==GroupRoleCYJ::Admin) m.role_=GroupRoleCYJ::Member; }
        pending_.clear();
    }
    policy_=std::move(p);
}
