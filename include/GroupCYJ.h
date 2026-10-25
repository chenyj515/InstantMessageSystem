#pragma once
#include "GroupMemberCYJ.h"
#include "GroupPolicyCYJ.h"
#include <map>
#include <set>
class GroupCYJ {
public:
    GroupCYJ(std::string id, std::string name, std::string owner, ServiceTypeCYJ service, std::unique_ptr<GroupPolicyCYJ> policy, std::string parent = {});
    const std::string& getGroupId() const;
    const std::string& getName() const;
    const std::string& getOwnerAccountId() const;
    const std::string& getParentGroupId() const;
    ServiceTypeCYJ getServiceType() const;
    const GroupPolicyCYJ& getPolicy() const;
    const std::map<std::string,GroupMemberCYJ>& getMembers() const;
    const std::set<std::string>& getPendingRequests() const;
    bool hasMember(const std::string& account) const;
    GroupRoleCYJ roleOf(const std::string& account) const;
private:
    friend class GroupManagerCYJ;
    friend class StorageManagerCYJ;
    void addMember(const std::string& account);
    void removeMember(const std::string& account);
    void setPolicy(std::unique_ptr<GroupPolicyCYJ> policy);
    std::string groupId_, name_, ownerAccountId_, parentGroupId_;
    ServiceTypeCYJ service_;
    std::map<std::string,GroupMemberCYJ> members_;
    std::set<std::string> pending_;
    std::unique_ptr<GroupPolicyCYJ> policy_;
};
