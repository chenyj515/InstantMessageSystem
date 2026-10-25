#pragma once
#include "FriendManagerCYJ.h"
#include "GroupManagerCYJ.h"
#include "LoginManagerCYJ.h"
#include "StorageManagerCYJ.h"
#include <functional>
#include <optional>
class PlatformControllerCYJ {
public:
    explicit PlatformControllerCYJ(std::filesystem::path directory = "data");
    bool initialize();
    void save() const;
    // Programmatic registration with an explicit unified user ID (tests / loaders only).
    // Interactive registration must use createUser(); never reuse a service number as user ID.
    void registerUser(const std::string& id,const std::string& name,const std::string& birth = {},const std::string& location = {});
    // Auto-generate a unique unified user ID (UID1, UID2, ...), create profile, return that ID.
    std::string createUser(const std::string& name,const std::string& birth = {},const std::string& location = {});
    std::string nextUserId() const;
    // Auto-generate a service number distinct from unified user IDs (QQ/Weibo: 10001+; WeChat: wx10001+).
    std::string nextServiceId(ServiceTypeCYJ type) const;
    void updateProfile(const std::string& name,const std::string& location,const std::string& birth);
    void openService(const std::string& user,ServiceTypeCYJ type,const std::string& serviceId);
    void bindQQ();
    void bindQQ(const std::string& userId);
    void login(const std::string& user,ServiceTypeCYJ type);
    void logout();
    // Online status (all opened services) is separate from the current operating service.
    const UserCYJ* getCurrentUser() const;
    bool hasCurrentService() const;
    ServiceTypeCYJ getCurrentService() const;
    const ServiceAccountCYJ& getCurrentAccount() const;
    std::string getCurrentAccountId() const;
    const UserCYJ& findUser(const std::string& id) const;
    const ServiceAccountCYJ& findAccount(const std::string& key) const;
    const std::map<std::string,UserCYJ>& getUsers() const;
    const FriendManagerCYJ& getFriendManager() const;
    const GroupManagerCYJ& getGroupManager() const;
    std::vector<std::string> getLoginStatus() const;
    void addFriend(const std::string& actor,const std::string& target,const std::string& remark = {});
    void updateFriend(const std::string& actor,const std::string& target,const std::string& remark);
    void removeFriend(const std::string& actor,const std::string& target);
    std::vector<FriendRelationCYJ> findFriends(const std::string& actor) const;
    std::vector<std::string> findCommonFriends(const std::string& a,const std::string& b) const;
    std::vector<std::string> recommendFriend(const std::string& targetAccount,ServiceTypeCYJ source) const;
    void createGroup(const std::string& id,const std::string& name,const std::string& owner);
    void joinGroup(const std::string& id,const std::string& account);
    void approveRequest(const std::string& id,const std::string& actor,const std::string& target,bool approve);
    void inviteMember(const std::string& id,const std::string& actor,const std::string& target);
    void leaveGroup(const std::string& id,const std::string& actor);
    void kickMember(const std::string& id,const std::string& actor,const std::string& target);
    void setAdmin(const std::string& id,const std::string& actor,const std::string& target,bool admin);
    void changeGroupPolicy(const std::string& id,const std::string& actor,const std::string& policy);
    void createSubgroup(const std::string& parent,const std::string& actor,const std::string& id,const std::string& name,const std::vector<std::string>& members);
    void dissolveGroup(const std::string& id,const std::string& actor);
    // UI commands execute in memory, then persist. Save failures retain dirty data for retry.
    void executeCommand(const std::function<void()>& command);
private:
    friend class StorageManagerCYJ;
    void authorize(const std::string& account) const;
    void requireGroupAccount(const std::string& group,const std::string& account) const;
    void validateAll() const;
    std::map<std::string,UserCYJ> users_;
    FriendManagerCYJ friendManager_;
    GroupManagerCYJ groupManager_;
    LoginManagerCYJ loginManager_;
    StorageManagerCYJ storageManager_;
    std::string currentUser_;
    std::optional<ServiceTypeCYJ> currentService_;
};
