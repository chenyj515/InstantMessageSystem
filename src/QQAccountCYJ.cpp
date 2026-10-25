#include "QQAccountCYJ.h"
QQAccountCYJ::QQAccountCYJ(const std::string& id, const std::string& user) : ServiceAccountCYJ(id, user) {}
ServiceTypeCYJ QQAccountCYJ::getServiceType() const { return ServiceTypeCYJ::QQ; }
