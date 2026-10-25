#pragma once
#include <filesystem>
class PlatformControllerCYJ;
class StorageManagerCYJ {
public:
    explicit StorageManagerCYJ(std::filesystem::path directory = "data");
    bool loadAll(PlatformControllerCYJ& platform) const;
    void saveAll(const PlatformControllerCYJ& platform) const;
    const std::filesystem::path& getDirectory() const;
private:
    std::filesystem::path directory_;
};
