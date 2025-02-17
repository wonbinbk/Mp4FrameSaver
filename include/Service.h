#ifndef SERVICE_H
#define SERVICE_H
#include <vector>
#include <cstdint>
#include <string>
#include <thread>

class Service {
public:
    Service(const std::string& queueName);
    virtual ~Service();

protected:
    virtual void processMessage(const std::vector<uint8_t>& message) = 0;
    void startThread();
    void waitThread();

private:
    void pollQueue();

    std::string mQueueName;
    int mDescriptor = -1;
    std::thread task;
};
#endif // SERVICE_H
