#pragma once

#include <drogon/drogon.h>
#include <functional>
#include <string>

namespace lepai {
namespace worker {
namespace messaging {

class TaskQueue {
public:
    using TaskCallback = std::function<void(const std::string& videoId)>;

    // 尝试获取一个任务
    void popTask(TaskCallback callback);
};

}
}
}