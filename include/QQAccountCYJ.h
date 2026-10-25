#pragma once
#include "ServiceAccountCYJ.h"
class QQAccountCYJ final : public ServiceAccountCYJ {
public:
    QQAccountCYJ(const std::string& id, const std::string& user);
    ServiceTypeCYJ getServiceType() const override;
};
