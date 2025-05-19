#pragma once 

#include <mutex>
#include <atomic>
#include <thread>

namespace ecs
{
    namespace profiling
    {
        namespace gui
        {
            struct shared_state_t 
            {
                std::mutex mutex;
                float backgroundColor[4] = {0.2f, 0.3f, 0.3f, 1.0f};
                bool quitApp = false;
            };

            class ProfilerGraphicsApp
            {
            public:
                void Open();
                void Close();
            private:
                void SetupWindow();
                void MainLoop(shared_state_t* sharedState);

                std::thread m_guiThread;
                std::atomic<bool> m_running{false};
                shared_state_t m_sharedState;
            };
        }
    }
}