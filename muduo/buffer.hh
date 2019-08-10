#ifndef BUFFER_HH
#define BUFFER_HH

#include <shared_mutex>
#include <string>
#include <vector>

class Buffer
{
public:
    const char *Peek() const;
    size_t ReadableBytes() const;
    void Retrieve(size_t len);
    void Append(const std::string &buf);
    std::string RetrieveAllAsString();
    std::string RetrieveAsString(size_t len);
    const char *Find(const std::string &str) const;

private:
    std::vector<char> buf_;
    mutable std::shared_mutex mutex_;
};

#endif