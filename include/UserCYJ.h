#pragma once
#include "ServiceAccountCYJ.h"
#include <map>
#include <memory>
class UserCYJ {
public:
    UserCYJ(std::string id, std::string nickname, std::string birth = {}, std::string location = {}, std::string registered = todayCYJ());
    const std::string& getUserId() const;
    const std::string& getNickname() const;
    const std::string& getBirthDate() const;
    const std::string& getRegisterTime() const;
    const std::string& getLocation() const;
    void updateProfile(const std::string& nickname, const std::string& location, const std::string& birth);
    const ServiceAccountCYJ* getAccount(ServiceTypeCYJ type) const;
    const std::map<ServiceTypeCYJ, std::unique_ptr<ServiceAccountCYJ>>& getAccounts() const;
private:
    friend class PlatformControllerCYJ;
    friend class LoginManagerCYJ;
    friend class StorageManagerCYJ;
    std::string userId_, nickname_, birthDate_, registerTime_, location_;
    std::map<ServiceTypeCYJ, std::unique_ptr<ServiceAccountCYJ>> accounts_;
};
