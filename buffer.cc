#include "buffer.hh"

using std::string;

const char *Buffer::Peek() const
{
    return buf_.data();
}

size_t Buffer::ReadableBytes() const
{
    return buf_.size();
}

void Buffer::Retrieve(size_t len)
{
    buf_ = std::vector<char>(buf_.begin() + len, buf_.end());
}

void Buffer::Append(const string &data)
{
    buf_.insert(buf_.end(), data.begin(), data.end());
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