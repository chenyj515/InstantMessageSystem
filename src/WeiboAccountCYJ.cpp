#include "WeiboAccountCYJ.h"
WeiboAccountCYJ::WeiboAccountCYJ(const std::string& id, const std::string& user) : ServiceAccountCYJ(id, user) {}
ServiceTypeCYJ WeiboAccountCYJ::getServiceType() const { return ServiceTypeCYJ::Weibo; }
