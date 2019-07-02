#include "http_request.hh"

using std::string;

void HttpRequest::AddHeader(const char *start, const char *colon, const char *end)
{
    string field(start, colon);
    ++colon;
    while (colon < end && isspace(*colon)) {
        ++colon;
    }
    string value(colon, end);
    while (!value.empty() && isspace(value[value.size() - 1])) {
        value.resize(value.size() - 1);
    }
    headers_[field] = value;
}

string HttpRequest::GetHeader(const string &field) const
{
    string result;
    auto it = headers_.find(field);
    if (it != headers_.end()) {
        result = it->second;
    }
    return result;
}

bool HttpRequest::SetMethod(const char *start, const char *end)
{
    string m(start, end);
    if (m == "GET") {
        method_ = kGet;
    } else if (m == "POST") {
        method_ = kPost;
    } else if (m == "HEAD") {
        method_ = kHead;
    } else if (m == "PUT") {
        method_ = kPut;
    } else if (m == "DELETE") {
        method_ = kDelete;
    } else {
        method_ = kInvalid;
    }
    return method_ != kInvalid;
}