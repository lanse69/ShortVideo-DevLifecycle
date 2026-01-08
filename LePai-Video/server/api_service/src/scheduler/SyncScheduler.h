#pragma once

#include <drogon/drogon.h>

namespace lepai {
namespace scheduler {

class SyncScheduler {
public:
    static void syncLikesToDB();
};

} // namespace scheduler
} // namespace lepai