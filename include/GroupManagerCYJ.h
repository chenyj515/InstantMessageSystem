#pragma once
#include "GroupCYJ.h"
#include <vector>
class GroupManagerCYJ {
public:
    const GroupCYJ& findGroup(const std::string& id) const;
    const std::map<std::string,GroupCYJ>& getGroups() const;
    void createGroup(const std::string& id,const std::string& name,const std::string& owner,ServiceTypeCYJ service);
    void joinGroup(const std::string& id,const std::string& account);
    void approveRequest(const std::string& id,const std::string& actor,const std::string& target,bool approve);
    void inviteMember(const std::string& id,const std::string& inviter,const std::string& target);
    void leaveGroup(const std::string& id,const std::string& account);
    void kickMember(const std::string& id,const std::string& actor,const std::string& target);
    void setAdmin(const std::string& id,const std::string& actor,const std::string& target,bool admin);
    void changeGroupPolicy(const std::string& id,const std::string& actor,const std::string& policy);
    void createSubgroup(const std::string& parent,const std::string& actor,const std::string& id,const std::string& name,const std::vector<std::string>& members);
    void dissolveGroup(const std::string& id,const std::string& actor);
private:
    friend class StorageManagerCYJ;
    GroupCYJ& mutableGroup(const std::string& id);
    std::map<std::string,GroupCYJ> groups_;
};
