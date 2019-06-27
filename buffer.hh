#ifndef BUFFER_HH
#define BUFFER_HH

#include <string>
#include <vector>

class Buffer
{
public:
    Buffer();
    ~Buffer();
    const char *Peek() const;
    int ReadableBytes() const;
    void Retrieve(int len);
    void Append(const std::string &buf);
    std::string RetrieveAllAsString();
    std::string RetrieveAsString(size_t len);

private:
    std::vector<char> buf_;
};

#endif