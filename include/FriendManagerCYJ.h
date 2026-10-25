#pragma once
#include "FriendRelationCYJ.h"
#include <map>
#include <vector>
class FriendManagerCYJ {
public:
    void addFriend(const std::string& account,const std::string& target,const std::string& remark = {});
    void removeFriend(const std::string& account,const std::string& target);
    void updateFriend(const std::string& account,const std::string& target,const std::string& remark);
    bool areFriends(const std::string& account,const std::string& target) const;
    std::vector<FriendRelationCYJ> findFriends(const std::string& account) const;
    std::vector<std::string> findCommonFriends(const std::string& a,const std::string& b) const;
    const std::map<std::pair<std::string,std::string>,FriendRelationCYJ>& getRelations() const;
private:
    friend class StorageManagerCYJ;
    std::map<std::pair<std::string,std::string>,FriendRelationCYJ> relations_;
};
