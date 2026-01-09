#include "utils.h"

#include <drogon/utils/Utilities.h>

namespace Utils {
    std::string generateUUID() {
        return drogon::utils::getUuid();
    }

    std::string hashPassword(const std::string& plainPassword) {
        return drogon::utils::getSha256(plainPassword);
    }
}