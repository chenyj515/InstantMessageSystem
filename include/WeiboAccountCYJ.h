#pragma once
#include "ServiceAccountCYJ.h"
class WeiboAccountCYJ final : public ServiceAccountCYJ {
public:
    WeiboAccountCYJ(const std::string& id, const std::string& user);
    ServiceTypeCYJ getServiceType() const override;
};
