#include "GroupManagerCYJ.h"
const GroupCYJ& GroupManagerCYJ::findGroup(const std::string& id) const { auto i=groups_.find(id); requireCYJ(i!=groups_.end(),"Group not found"); return i->second; }
GroupCYJ& GroupManagerCYJ::mutableGroup(const std::string& id) { return const_cast<GroupCYJ&>(static_cast<const GroupManagerCYJ&>(*this).findGroup(id)); }
const std::map<std::string,GroupCYJ>& GroupManagerCYJ::getGroups() const { return groups_; }
void GroupManagerCYJ::createGroup(const std::string& id,const std::string& name,const std::string& owner,ServiceTypeCYJ t) {
    requireCYJ(groups_.count(id)==0,"Group ID exists"); groups_.emplace(id,GroupCYJ(id,name,owner,t,GroupPolicyCYJ::create(toStringCYJ(t))));
}
void GroupManagerCYJ::joinGroup(const std::string& id,const std::string& account) {
    auto& g=mutableGroup(id); requireCYJ(g.getPolicy().getName()=="QQ" && g.getParentGroupId().empty(),"This group requires an invitation");
    requireCYJ(!g.hasMember(account),"Already in group"); requireCYJ(g.pending_.insert(account).second,"Application already pending");
}
void GroupManagerCYJ::approveRequest(const std::string& id,const std::string& actor,const std::string& target,bool approve) {
    auto& g=mutableGroup(id); const auto r=g.roleOf(actor);
    requireCYJ(g.getPolicy().getName()=="QQ" && (r==GroupRoleCYJ::Owner || r==GroupRoleCYJ::Admin),"No permission to process requests");
    requireCYJ(g.pending_.count(target)!=0,"No pending application");
    if(approve) { requireCYJ(g.getPolicy().canJoin(true,false),"Join forbidden"); g.addMember(target); } else g.pending_.erase(target);
}
void GroupManagerCYJ::inviteMember(const std::string& id,const std::string& inviter,const std::string& target) {
    auto& g=mutableGroup(id); g.roleOf(inviter);
    requireCYJ(g.getPolicy().canJoin(false,true),"Current group policy does not allow invitation"); g.addMember(target);
}
void GroupManagerCYJ::leaveGroup(const std::string& id,const std::string& a) { mutableGroup(id).removeMember(a); }
void GroupManagerCYJ::kickMember(const std::string& id,const std::string& a,const std::string& b) {
    auto& g=mutableGroup(id); requireCYJ(g.getPolicy().canKick(g.roleOf(a),g.roleOf(b)),"No permission to kick this member"); g.removeMember(b);
}
void GroupManagerCYJ::setAdmin(const std::string& id,const std::string& a,const std::string& b,bool admin) {
    auto& g=mutableGroup(id); requireCYJ(g.getPolicy().canManageAdmin(g.roleOf(a)),"Cannot manage administrators");
    requireCYJ(g.roleOf(b)!=GroupRoleCYJ::Owner,"Cannot change owner's role"); g.members_.at(b).role_=admin?GroupRoleCYJ::Admin:GroupRoleCYJ::Member;
}
void GroupManagerCYJ::changeGroupPolicy(const std::string& id,const std::string& actor,const std::string& policy) {
    auto& g=mutableGroup(id); requireCYJ(actor==g.getOwnerAccountId(),"Only the owner can switch policy");
    requireCYJ(g.getParentGroupId().empty() || policy=="QQ","Discussion group uses QQ policy"); g.setPolicy(GroupPolicyCYJ::create(policy));
}
void GroupManagerCYJ::createSubgroup(const std::string& parent,const std::string& actor,const std::string& id,const std::string& name,const std::vector<std::string>& members) {
    const auto& p=findGroup(parent); requireCYJ(p.getPolicy().canCreateSubgroup(p.roleOf(actor)),"Cannot create discussion group");
    requireCYJ(p.getParentGroupId().empty(),"Nested discussion groups are unsupported"); requireCYJ(groups_.count(id)==0,"Group ID exists");
    GroupCYJ child(id,name,actor,p.getServiceType(),GroupPolicyCYJ::create("QQ"),parent);
    for(const auto& a:members) { requireCYJ(p.hasMember(a),"Discussion member must belong to parent group"); if(!child.hasMember(a)) child.addMember(a); }
    groups_.emplace(id,std::move(child));
}
void GroupManagerCYJ::dissolveGroup(const std::string& id,const std::string& actor) {
    requireCYJ(findGroup(id).getOwnerAccountId()==actor,"Only the owner can dissolve a group");
    for(auto i=groups_.begin();i!=groups_.end();) { if(i->second.getParentGroupId()==id) i=groups_.erase(i); else ++i; } groups_.erase(id);
}
