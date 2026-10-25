#include "FriendManagerCYJ.h"
#include "TypesCYJ.h"
#include <set>
void FriendManagerCYJ::addFriend(const std::string& a,const std::string& b,const std::string& remark) {
    requireCYJ(a!=b,"Cannot add yourself"); requireCYJ(!areFriends(a,b),"Friend already exists");
    FriendRelationCYJ forward(a,b,remark), reverse(b,a);
    relations_.emplace(std::make_pair(a,b),std::move(forward));
    try { relations_.emplace(std::make_pair(b,a),std::move(reverse)); }
    catch (...) { relations_.erase({a,b}); throw; }
}
void FriendManagerCYJ::removeFriend(const std::string& a,const std::string& b) { requireCYJ(areFriends(a,b),"Friend not found"); relations_.erase({a,b}); relations_.erase({b,a}); }
void FriendManagerCYJ::updateFriend(const std::string& a,const std::string& b,const std::string& r) { requireCYJ(areFriends(a,b),"Friend not found"); relations_.at({a,b}).updateRemark(r); }
bool FriendManagerCYJ::areFriends(const std::string& a,const std::string& b) const { return relations_.count({a,b})!=0; }
std::vector<FriendRelationCYJ> FriendManagerCYJ::findFriends(const std::string& a) const { std::vector<FriendRelationCYJ> out; for(const auto& [key,r]:relations_) if(key.first==a) out.push_back(r); return out; }
std::vector<std::string> FriendManagerCYJ::findCommonFriends(const std::string& a,const std::string& b) const {
    std::set<std::string> first; for(const auto& r:findFriends(a)) first.insert(r.getFriendAccountId());
    std::vector<std::string> out; for(const auto& r:findFriends(b)) if(first.count(r.getFriendAccountId())) out.push_back(r.getFriendAccountId()); return out;
}
const std::map<std::pair<std::string,std::string>,FriendRelationCYJ>& FriendManagerCYJ::getRelations() const { return relations_; }
