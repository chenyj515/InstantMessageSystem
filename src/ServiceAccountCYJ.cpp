#include "ServiceAccountCYJ.h"
#include <utility>
ServiceAccountCYJ::ServiceAccountCYJ(std::string id, std::string user) : serviceId_(std::move(id)), userId_(std::move(user)) { validateIdCYJ(serviceId_); validateIdCYJ(userId_); }
ServiceAccountCYJ::~ServiceAccountCYJ() = default;
const std::string& ServiceAccountCYJ::getServiceId() const { return serviceId_; }
const std::string& ServiceAccountCYJ::getUserId() const { return userId_; }
std::string accountKeyCYJ(ServiceTypeCYJ t, const std::string& id) { return toStringCYJ(t) + ":" + id; }
std::string ServiceAccountCYJ::getAccountId() const { return accountKeyCYJ(getServiceType(), serviceId_); }
bool ServiceAccountCYJ::isLoggedIn() const { return loggedIn_; }
bool ServiceAccountCYJ::login() { loggedIn_ = true; return true; }
void ServiceAccountCYJ::logout() { loggedIn_ = false; }
