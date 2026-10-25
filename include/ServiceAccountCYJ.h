#pragma once
#include "TypesCYJ.h"

class ServiceAccountCYJ {
public:
    ServiceAccountCYJ(std::string serviceId, std::string userId);
    virtual ~ServiceAccountCYJ();
    virtual ServiceTypeCYJ getServiceType() const = 0;
    const std::string& getServiceId() const;
    const std::string& getUserId() const;
    std::string getAccountId() const;
    bool isLoggedIn() const;
    bool login();
    void logout();
private:
    std::string serviceId_;
    std::string userId_;
    bool loggedIn_ = false;
};
