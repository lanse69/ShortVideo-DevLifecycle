#pragma once

#include <memory>

#include "../messaging/TaskQueue.h"
#include "../repository/VideoStatusRepository.h"
#include "MinioClient.h"

namespace lepai {
namespace worker {
namespace service {

class TranscodeService {
public:
    TranscodeService();
    void start();

private:
    void processTask(const std::string& videoId);

    std::shared_ptr<messaging::TaskQueue> queue;
    std::shared_ptr<repository::VideoStatusRepository> repository;
    std::unique_ptr<lepai::storage::MinioClient> storage;
};

}
}
}