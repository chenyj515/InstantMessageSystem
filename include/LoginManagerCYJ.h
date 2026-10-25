#pragma once
#include "UserCYJ.h"
#include <vector>
class LoginManagerCYJ {
public:
    void loginService(UserCYJ& user, ServiceTypeCYJ selected);
    void logoutAll(UserCYJ& user);
    std::vector<std::string> getLoginStatus(const UserCYJ& user) const;
};
