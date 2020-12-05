#ifndef ANBOX_ANDROID_CHROME_CONNECTOR_H_
#define ANBOX_ANDROID_CHROME_CONNECTOR_H_

#include <memory>
#include <thread>
#include <atomic>

namespace anbox {

namespace rpc {
class PendingCallCache;
class Channel;
} // namespace rpc

class LocalSocketConnection;
class ChromeMessageProcessor;
class AndroidApiSkeleton;
class PlatformApiStub;

class ChromeConnector {
public:
    ChromeConnector();
    ~ChromeConnector();

    void start();
    void stop();

    std::shared_ptr<rpc::Channel> get_rpc_channel() const { return rpc_channel_; }    

private:
    void main_loop();

    std::shared_ptr<LocalSocketConnection> socket_;
    std::shared_ptr<rpc::PendingCallCache> pending_calls_;
    std::shared_ptr<AndroidApiSkeleton> android_api_skeleton_;
    std::shared_ptr<ChromeMessageProcessor> message_processor_;
    std::shared_ptr<rpc::Channel> rpc_channel_;    
    std::thread thread_;
    std::atomic<bool> running_;
};
} // namespace anbox

#endif