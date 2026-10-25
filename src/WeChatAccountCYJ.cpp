#include "WeChatAccountCYJ.h"
WeChatAccountCYJ::WeChatAccountCYJ(const std::string& id, const std::string& user) : ServiceAccountCYJ(id, user) {}
ServiceTypeCYJ WeChatAccountCYJ::getServiceType() const { return ServiceTypeCYJ::WeChat; }
void WeChatAccountCYJ::bindQQ(const ServiceAccountCYJ& qq) {
    requireCYJ(qq.getServiceType()==ServiceTypeCYJ::QQ && qq.getUserId()==getUserId(), "Only your own existing QQ account can be bound");
    boundQQId_=qq.getServiceId();
}
const std::string& WeChatAccountCYJ::getBoundQQId() const { return boundQQId_; }
