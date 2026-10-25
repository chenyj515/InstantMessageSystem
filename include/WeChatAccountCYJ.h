#pragma once
#include "ServiceAccountCYJ.h"
class WeChatAccountCYJ final : public ServiceAccountCYJ {
public:
    WeChatAccountCYJ(const std::string& id, const std::string& user);
    ServiceTypeCYJ getServiceType() const override;
    void bindQQ(const ServiceAccountCYJ& qq);
    const std::string& getBoundQQId() const;
private:
    std::string boundQQId_;
};
