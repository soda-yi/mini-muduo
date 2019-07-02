#include "buffer.hh"

#include <algorithm>

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
    len = std::min(len, buf_.size());
    string result(Peek(), len);
    Retrieve(len);
    return result;
}

const char *Buffer::Find(const std::string &str) const
{
    auto it = std::search(buf_.cbegin(), buf_.cend(), str.begin(), str.end());
    return it == buf_.end() ? nullptr : &*it;
}