#include "PlatformControllerCYJ.h"
#include "QQAccountCYJ.h"
#include "WeChatAccountCYJ.h"
#include "WeiboAccountCYJ.h"
#include <algorithm>
#include <set>

PlatformControllerCYJ::PlatformControllerCYJ(std::filesystem::path p):storageManager_(std::move(p)) {}
bool PlatformControllerCYJ::initialize() { return storageManager_.loadAll(*this); }
void PlatformControllerCYJ::save() const { storageManager_.saveAll(*this); }
namespace {
bool allDigitsCYJ(const std::string& s) {
    if(s.empty()) return false;
    for(unsigned char c:s) if(c<'0'||c>'9') return false;
    return true;
}
bool serviceIdUsedCYJ(const std::map<std::string,UserCYJ>& users,ServiceTypeCYJ type,const std::string& sid) {
    for(const auto& [uid,user]:users) {
        (void)uid;
        if(type==ServiceTypeCYJ::WeChat) {
            if(const auto* a=user.getAccount(ServiceTypeCYJ::WeChat); a && a->getServiceId()==sid) return true;
        } else {
            for(auto kind:{ServiceTypeCYJ::QQ,ServiceTypeCYJ::Weibo}) {
                if(const auto* a=user.getAccount(kind); a && a->getServiceId()==sid) return true;
            }
        }
    }
    return false;
}
}
void PlatformControllerCYJ::registerUser(const std::string& id,const std::string& n,const std::string& b,const std::string& l) {
    requireCYJ(users_.count(id)==0,"User ID exists");
    for(const auto& [uid,user]:users_) {
        (void)uid;
        for(const auto& [t,a]:user.getAccounts()) {
            (void)t;
            requireCYJ(a->getServiceId()!=id,"Unified user ID must not equal an existing service number");
        }
    }
    users_.emplace(id,UserCYJ(id,n,b,l));
}
std::string PlatformControllerCYJ::nextUserId() const {
    int max=0;
    for(const auto& [id,u]:users_) {
        (void)u;
        if(id.rfind("UID",0)!=0) continue;
        const auto num=id.substr(3);
        if(!allDigitsCYJ(num)) continue;
        max=std::max(max,std::stoi(num));
    }
    std::string candidate;
    do { ++max; candidate="UID"+std::to_string(max); } while(users_.count(candidate) || serviceIdUsedCYJ(users_,ServiceTypeCYJ::QQ,candidate) || serviceIdUsedCYJ(users_,ServiceTypeCYJ::WeChat,candidate));
    return candidate;
}
std::string PlatformControllerCYJ::nextServiceId(ServiceTypeCYJ type) const {
    if(type==ServiceTypeCYJ::WeChat) {
        int max=10000;
        for(const auto& [uid,user]:users_) {
            (void)uid;
            const auto* a=user.getAccount(ServiceTypeCYJ::WeChat); if(!a) continue;
            const auto& sid=a->getServiceId();
            if(sid.rfind("wx",0)==0 && allDigitsCYJ(sid.substr(2))) max=std::max(max,std::stoi(sid.substr(2)));
        }
        std::string candidate;
        do { ++max; candidate="wx"+std::to_string(max); } while(users_.count(candidate) || serviceIdUsedCYJ(users_,ServiceTypeCYJ::WeChat,candidate));
        return candidate;
    }
    int max=10000;
    for(const auto& [uid,user]:users_) {
        (void)uid;
        for(auto kind:{ServiceTypeCYJ::QQ,ServiceTypeCYJ::Weibo}) {
            const auto* a=user.getAccount(kind); if(!a) continue;
            if(allDigitsCYJ(a->getServiceId())) max=std::max(max,std::stoi(a->getServiceId()));
        }
    }
    std::string candidate;
    do { ++max; candidate=std::to_string(max); } while(users_.count(candidate) || serviceIdUsedCYJ(users_,ServiceTypeCYJ::QQ,candidate));
    return candidate;
}
std::string PlatformControllerCYJ::createUser(const std::string& n,const std::string& b,const std::string& l) {
    const auto id=nextUserId(); registerUser(id,n,b,l); return id;
}
void PlatformControllerCYJ::updateProfile(const std::string& n,const std::string& l,const std::string& b) { requireCYJ(getCurrentUser()!=nullptr,"Login first"); users_.at(currentUser_).updateProfile(n,l,b); }
const UserCYJ& PlatformControllerCYJ::findUser(const std::string& id) const { auto i=users_.find(id); requireCYJ(i!=users_.end(),"User not found"); return i->second; }
const ServiceAccountCYJ& PlatformControllerCYJ::findAccount(const std::string& key) const {
    for(const auto& [id,u]:users_) { (void)id; for(const auto& [t,a]:u.getAccounts()) { (void)t; if(a->getAccountId()==key) return *a; } }
    throw std::runtime_error("Account not found: "+key);
}
void PlatformControllerCYJ::openService(const std::string& id,ServiceTypeCYJ t,const std::string& sid) {
    findUser(id); requireCYJ(currentUser_.empty() || currentUser_==id,"Cannot activate services for another user while logged in");
    auto& u=users_.at(id); requireCYJ(!u.getAccount(t),"Service already activated"); validateIdCYJ(sid);
    // Unified user ID and service numbers are different layers and must not share the same string.
    requireCYJ(sid!=id,"Service number must differ from unified user ID");
    requireCYJ(users_.count(sid)==0,"Service number must not equal any unified user ID");
    if(t==ServiceTypeCYJ::WeChat) {
        requireCYJ(!serviceIdUsedCYJ(users_,ServiceTypeCYJ::QQ,sid),"WeChat number must differ from QQ/Weibo numbers");
    } else {
        requireCYJ(!serviceIdUsedCYJ(users_,ServiceTypeCYJ::WeChat,sid),"QQ/Weibo number must differ from WeChat numbers");
        const auto other=t==ServiceTypeCYJ::QQ?ServiceTypeCYJ::Weibo:ServiceTypeCYJ::QQ;
        if(const auto* a=u.getAccount(other)) requireCYJ(a->getServiceId()==sid,"QQ and Weibo must share the same ID");
        // Reserve the shared QQ/Weibo number across users even if one service is unopened.
        for(const auto& [uid,user]:users_) if(uid!=id) for(auto kind:{ServiceTypeCYJ::QQ,ServiceTypeCYJ::Weibo}) {
            const auto* a=user.getAccount(kind); requireCYJ(!a || a->getServiceId()!=sid,"Shared QQ/Weibo ID belongs to another user");
        }
    }
    for(const auto& [uid,user]:users_) { (void)uid; const auto* a=user.getAccount(t); requireCYJ(!a || a->getServiceId()!=sid,"Service ID exists"); }
    std::unique_ptr<ServiceAccountCYJ> account;
    switch(t) {
        case ServiceTypeCYJ::QQ: account=std::make_unique<QQAccountCYJ>(sid,id); break;
        case ServiceTypeCYJ::WeChat: account=std::make_unique<WeChatAccountCYJ>(sid,id); break;
        case ServiceTypeCYJ::Weibo: account=std::make_unique<WeiboAccountCYJ>(sid,id); break;
    }
    requireCYJ(account!=nullptr,"Invalid service"); if(currentUser_==id) account->login(); u.accounts_.emplace(t,std::move(account));
}
void PlatformControllerCYJ::bindQQ() {
    requireCYJ(getCurrentUser()!=nullptr,"Login first"); bindQQ(currentUser_);
}
void PlatformControllerCYJ::bindQQ(const std::string& userId) {
    findUser(userId); auto& u=users_.at(userId);
    auto i=u.accounts_.find(ServiceTypeCYJ::WeChat); const auto* qq=u.getAccount(ServiceTypeCYJ::QQ);
    requireCYJ(i!=u.accounts_.end() && qq,"Activate both QQ and WeChat first"); dynamic_cast<WeChatAccountCYJ&>(*i->second).bindQQ(*qq);
}
void PlatformControllerCYJ::login(const std::string& id,ServiceTypeCYJ t) {
    findUser(id); requireCYJ(findUser(id).getAccount(t)!=nullptr,"Selected service is not activated");
    logout(); loginManager_.loginService(users_.at(id),t); currentUser_=id; currentService_=t;
}
void PlatformControllerCYJ::logout() {
    if(!currentUser_.empty()) loginManager_.logoutAll(users_.at(currentUser_));
    currentUser_.clear(); currentService_.reset();
}
const UserCYJ* PlatformControllerCYJ::getCurrentUser() const { return currentUser_.empty()?nullptr:&users_.at(currentUser_); }
bool PlatformControllerCYJ::hasCurrentService() const { return currentService_.has_value() && getCurrentUser()!=nullptr; }
ServiceTypeCYJ PlatformControllerCYJ::getCurrentService() const {
    requireCYJ(hasCurrentService(),"Login and select an operating service first"); return *currentService_;
}
const ServiceAccountCYJ& PlatformControllerCYJ::getCurrentAccount() const {
    const auto* u=getCurrentUser(); requireCYJ(u && currentService_,"Login and select an operating service first");
    const auto* a=u->getAccount(*currentService_); requireCYJ(a,"Current operating service is not activated"); return *a;
}
std::string PlatformControllerCYJ::getCurrentAccountId() const { return getCurrentAccount().getAccountId(); }
const std::map<std::string,UserCYJ>& PlatformControllerCYJ::getUsers() const { return users_; }
const FriendManagerCYJ& PlatformControllerCYJ::getFriendManager() const { return friendManager_; }
const GroupManagerCYJ& PlatformControllerCYJ::getGroupManager() const { return groupManager_; }
std::vector<std::string> PlatformControllerCYJ::getLoginStatus() const { requireCYJ(getCurrentUser()!=nullptr,"Login first"); return loginManager_.getLoginStatus(*getCurrentUser()); }
void PlatformControllerCYJ::authorize(const std::string& a) const { const auto& x=findAccount(a); requireCYJ(!currentUser_.empty() && x.getUserId()==currentUser_ && x.isLoggedIn(),"Operate only your own logged-in account"); }
void PlatformControllerCYJ::addFriend(const std::string& a,const std::string& b,const std::string& r) {
    authorize(a); requireCYJ(findAccount(a).getServiceType()==findAccount(b).getServiceType(),"Friend accounts must use the same service"); friendManager_.addFriend(a,b,r);
}
void PlatformControllerCYJ::updateFriend(const std::string& a,const std::string& b,const std::string& r) { authorize(a); friendManager_.updateFriend(a,b,r); }
void PlatformControllerCYJ::removeFriend(const std::string& a,const std::string& b) { authorize(a); friendManager_.removeFriend(a,b); }
std::vector<FriendRelationCYJ> PlatformControllerCYJ::findFriends(const std::string& a) const { authorize(a); return friendManager_.findFriends(a); }
std::vector<std::string> PlatformControllerCYJ::findCommonFriends(const std::string& a,const std::string& b) const {
    authorize(a); requireCYJ(findAccount(a).getServiceType()==findAccount(b).getServiceType(),"Use the same service"); return friendManager_.findCommonFriends(a,b);
}
std::vector<std::string> PlatformControllerCYJ::recommendFriend(const std::string& target,ServiceTypeCYJ source) const {
    authorize(target); const auto& dst=findAccount(target); const auto& user=findUser(dst.getUserId()); const auto* src=user.getAccount(source);
    requireCYJ(src!=nullptr && source!=dst.getServiceType(),"Activate a different source service first");
    if(dst.getServiceType()==ServiceTypeCYJ::WeChat && source==ServiceTypeCYJ::QQ) {
        const auto& wx=dynamic_cast<const WeChatAccountCYJ&>(dst); requireCYJ(wx.getBoundQQId()==src->getServiceId(),"Bind QQ before requesting QQ recommendations");
    }
    std::set<std::string> result;
    for(const auto& relation:friendManager_.findFriends(src->getAccountId())) {
        const auto& other=findUser(findAccount(relation.getFriendAccountId()).getUserId()); const auto* candidate=other.getAccount(dst.getServiceType());
        if(candidate && candidate->getAccountId()!=target && !friendManager_.areFriends(target,candidate->getAccountId())) result.insert(candidate->getAccountId());
    }
    return {result.begin(),result.end()};
}
void PlatformControllerCYJ::requireGroupAccount(const std::string& id,const std::string& a) const {
    requireCYJ(groupManager_.findGroup(id).getServiceType()==findAccount(a).getServiceType(),"Account must use the group's original service");
}
void PlatformControllerCYJ::createGroup(const std::string& id,const std::string& name,const std::string& owner) { authorize(owner); groupManager_.createGroup(id,name,owner,findAccount(owner).getServiceType()); }
void PlatformControllerCYJ::joinGroup(const std::string& id,const std::string& a) { authorize(a); requireGroupAccount(id,a); groupManager_.joinGroup(id,a); }
void PlatformControllerCYJ::approveRequest(const std::string& id,const std::string& a,const std::string& b,bool yes) { authorize(a); requireGroupAccount(id,b); groupManager_.approveRequest(id,a,b,yes); }
void PlatformControllerCYJ::inviteMember(const std::string& id,const std::string& a,const std::string& b) {
    authorize(a); requireGroupAccount(id,b); requireCYJ(friendManager_.areFriends(a,b),"Only an existing friend can be recommended"); groupManager_.inviteMember(id,a,b);
}
void PlatformControllerCYJ::leaveGroup(const std::string& id,const std::string& a) { authorize(a); groupManager_.leaveGroup(id,a); }
void PlatformControllerCYJ::kickMember(const std::string& id,const std::string& a,const std::string& b) { authorize(a); groupManager_.kickMember(id,a,b); }
void PlatformControllerCYJ::setAdmin(const std::string& id,const std::string& a,const std::string& b,bool yes) { authorize(a); groupManager_.setAdmin(id,a,b,yes); }
void PlatformControllerCYJ::changeGroupPolicy(const std::string& id,const std::string& a,const std::string& p) { authorize(a); groupManager_.changeGroupPolicy(id,a,p); }
void PlatformControllerCYJ::createSubgroup(const std::string& p,const std::string& a,const std::string& id,const std::string& n,const std::vector<std::string>& m) { authorize(a); groupManager_.createSubgroup(p,a,id,n,m); }
void PlatformControllerCYJ::dissolveGroup(const std::string& id,const std::string& a) { authorize(a); groupManager_.dissolveGroup(id,a); }
void PlatformControllerCYJ::executeCommand(const std::function<void()>& command) {
    command();
    try { save(); } catch(const std::exception& e) { throw std::runtime_error(std::string("Memory updated, but SAVE FAILED. Retry Save before exit: ")+e.what()); }
}
void PlatformControllerCYJ::validateAll() const {
    std::set<std::string> keys; std::map<std::string,std::string> shared;
    for(const auto& [uid,u]:users_) for(const auto& [t,a]:u.getAccounts()) {
        requireCYJ(a->getUserId()==uid && a->getServiceType()==t && keys.insert(a->getAccountId()).second,"Invalid or duplicate account");
        if(t!=ServiceTypeCYJ::WeChat) { auto [i,inserted]=shared.emplace(a->getServiceId(),uid); requireCYJ(inserted || i->second==uid,"Shared ID collision"); }
        if(t==ServiceTypeCYJ::WeChat) { const auto& wx=dynamic_cast<const WeChatAccountCYJ&>(*a); const auto* qq=u.getAccount(ServiceTypeCYJ::QQ); requireCYJ(wx.getBoundQQId().empty() || (qq && qq->getServiceId()==wx.getBoundQQId()),"Invalid QQ binding"); }
        const auto* qq=u.getAccount(ServiceTypeCYJ::QQ); const auto* wb=u.getAccount(ServiceTypeCYJ::Weibo);
        requireCYJ(!qq || !wb || qq->getServiceId()==wb->getServiceId(),"QQ/Weibo ID mismatch");
    }
    for(const auto& [key,r]:friendManager_.getRelations()) {
        requireCYJ(key.first!=key.second && findAccount(key.first).getServiceType()==findAccount(key.second).getServiceType(),"Invalid friendship");
        requireCYJ(friendManager_.areFriends(key.second,key.first),"Asymmetric friendship"); validateTextCYJ(r.getRemark(),"Remark",true);
    }
    for(const auto& [id,g]:groupManager_.getGroups()) {
        requireCYJ(g.hasMember(g.getOwnerAccountId()) && g.roleOf(g.getOwnerAccountId())==GroupRoleCYJ::Owner,"Missing group owner");
        for(const auto& [a,m]:g.getMembers()) {
            requireGroupAccount(id,a); requireCYJ(m.getRole()!=GroupRoleCYJ::Owner || a==g.getOwnerAccountId(),"Multiple owners");
            requireCYJ(g.getPolicy().getName()=="QQ" || m.getRole()!=GroupRoleCYJ::Admin,"WeChat policy cannot retain admins");
        }
        for(const auto& a:g.getPendingRequests()) { requireGroupAccount(id,a); requireCYJ(!g.hasMember(a) && g.getPolicy().getName()=="QQ" && g.getParentGroupId().empty(),"Invalid pending request"); }
        if(!g.getParentGroupId().empty()) {
            const auto& p=groupManager_.findGroup(g.getParentGroupId());
            requireCYJ(p.getParentGroupId().empty() && g.getParentGroupId()!=id && p.getServiceType()==g.getServiceType() && g.getPolicy().getName()=="QQ","Invalid discussion parent");
        }
    }
}
