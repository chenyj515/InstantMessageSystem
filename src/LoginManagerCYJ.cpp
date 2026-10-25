#include "LoginManagerCYJ.h"
void LoginManagerCYJ::loginService(UserCYJ& u,ServiceTypeCYJ t) {
    requireCYJ(u.getAccount(t)!=nullptr,"Selected service is not activated");
    for(auto& [type,a]:u.accounts_) { (void)type; a->login(); }
}
void LoginManagerCYJ::logoutAll(UserCYJ& u) { for(auto& [type,a]:u.accounts_) { (void)type; a->logout(); } }
std::vector<std::string> LoginManagerCYJ::getLoginStatus(const UserCYJ& u) const {
    std::vector<std::string> out; for(const auto& [t,a]:u.getAccounts()) { (void)t; out.push_back(a->getAccountId()+(a->isLoggedIn()?" online":" offline")); } return out;
}
