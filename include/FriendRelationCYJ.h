#pragma once
#include <string>
class FriendRelationCYJ {
public:
    FriendRelationCYJ(std::string account, std::string target, std::string remark = {});
    const std::string& getAccountId() const;
    const std::string& getFriendAccountId() const;
    const std::string& getRemark() const;
    void updateRemark(const std::string& remark);
private:
    std::string accountId_, friendAccountId_, remark_;
};
