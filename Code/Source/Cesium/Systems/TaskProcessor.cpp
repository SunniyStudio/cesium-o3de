#include "Cesium/Systems/TaskProcessor.h"
#include <AzCore/Jobs/JobFunction.h>

namespace Cesium
{
    TaskProcessor::TaskProcessor()
    {
        AZ::JobManagerDesc jobDesc;
        size_t numThreads = AZStd::thread::hardware_concurrency();
        if (numThreads == 0)
        {
            numThreads = 2;
        }
        for (size_t i = 0; i < numThreads; ++i)
        {
            jobDesc.m_workerThreads.push_back({ static_cast<int>(i) });
        }
        m_jobManager = AZStd::make_unique<AZ::JobManager>(jobDesc);
        m_jobContext = AZStd::make_unique<AZ::JobContext>(*m_jobManager);
    }

    TaskProcessor::~TaskProcessor() noexcept
    {
        m_jobContext.reset();
        m_jobManager.reset();
    }

    void TaskProcessor::startTask(std::function<void()> task)
    {
        AZ::Job* job = aznew AZ::JobFunction<std::function<void()>>(std::move(task), true, m_jobContext.get());
        job->Start();
    }
} // namespace Cesium
