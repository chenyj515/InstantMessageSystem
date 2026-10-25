#include "GroupPolicyCYJ.h"
#include "QQGroupPolicyCYJ.h"
#include "WeChatGroupPolicyCYJ.h"
GroupPolicyCYJ::~GroupPolicyCYJ() = default;
std::unique_ptr<GroupPolicyCYJ> GroupPolicyCYJ::create(const std::string& n) {
    if(n=="QQ") return std::make_unique<QQGroupPolicyCYJ>();
    if(n=="WeChat") return std::make_unique<WeChatGroupPolicyCYJ>();
    throw std::runtime_error("Group policy must be QQ or WeChat");
}
