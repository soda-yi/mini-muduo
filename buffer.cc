#include "buffer.hh"

using std::string;

Buffer::Buffer()
{
}

Buffer::~Buffer()
{
}

const char *Buffer::Peek() const
{
    return buf_.data();
}

int Buffer::ReadableBytes() const
{
    return static_cast<int>(buf_.size());
}

void Buffer::Retrieve(int len)
{
    buf_ = std::vector<char>(buf_.begin() + len, buf_.end());
}

void Buffer::Append(const string &data)
{
    buf_.emplace_back(data.begin(), data.end());
}

string Buffer::RetrieveAllAsString()
{
    return RetrieveAsString(ReadableBytes());
}

string Buffer::RetrieveAsString(size_t len)
{
    string result(Peek(), len);
    Retrieve(len);
    return result;
}