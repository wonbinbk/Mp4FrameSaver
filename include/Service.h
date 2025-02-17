#ifndef SERVICE_H
#define SERVICE_H
#include <vector>
#include <cstdint>
#include <string>

class Service {
public:
  Service(const std::string &queueName);
  virtual ~Service();

protected:
  virtual void processMessage(const std::vector<uint8_t> &message);

private:
  void pollQueue();
  std::string mQueueName;
  int mDescriptor = -1;
};
#endif // SERVICE_H
