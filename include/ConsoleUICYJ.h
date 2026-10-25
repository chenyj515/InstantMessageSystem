#pragma once
#include "PlatformControllerCYJ.h"
#include <iostream>
#include <vector>
class ConsoleUICYJ {
public:
    explicit ConsoleUICYJ(PlatformControllerCYJ& controller,std::istream& input = std::cin,std::ostream& output = std::cout);
    void run();
    void showMainMenu();
    void showFriendMenu();
    void showGroupMenu();
private:
    std::string read(const std::string& prompt);
    int choice(const std::string& prompt);
    bool askYes(const std::string& prompt);
    // Resolve peer id on the current operating service (service number or full account key).
    std::string peerOnCurrentService(const std::string& prompt);
    ServiceTypeCYJ pickOtherOpenedService(const std::string& prompt);
    // Print numbered contact list for the current service; returns the same ordered list.
    std::vector<FriendRelationCYJ> showContacts(const std::string& selfAccount);
    void approvePendingWizard(const std::string& actor);
    void inviteFriendToGroupWizard(const std::string& actor);
    void kickMemberWizard(const std::string& actor);
    void setAdminWizard(const std::string& actor);
    void changePolicyWizard(const std::string& actor);
    void createSubgroupWizard(const std::string& actor);
    void registerWizard();
    void openServiceWizard(const std::string& userId,bool duringRegister);
    void loginWizard();
    void showUsers();
    void showGroups();
    void showMembers();
    void showLoginStatus();
    PlatformControllerCYJ& controller_;
    std::istream& input_;
    std::ostream& output_;
};
