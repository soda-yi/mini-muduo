#ifndef SAMPLE_HTTP_HTTP_REQUEST_HH
#define SAMPLE_HTTP_HTTP_REQUEST_HH

#include <chrono>
#include <iostream>
#include <map>
#include <string>

class HttpRequest
{
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    enum Method {
        kInvalid,
        kGet,
        kPost,
        kHead,
        kPut,
        kDelete
    };
    enum Version {
        kUnknown,
        kHttp10,
        kHttp11
    };

    HttpRequest()
        : method_(kInvalid), version_(kUnknown)
    {
    }

    void SetVersion(Version v) { version_ = v; }
    Version GetVersion() const { return version_; }
    bool SetMethod(const char *start, const char *end);
    Method GetMethod() const { return method_; }
    void SetPath(const char *start, const char *end) { path_.assign(start, end); }
    const std::string &GetPath() const { return path_; }
    void SetQuery(const char *start, const char *end) { query_.assign(start, end); }
    const std::string &GetQuery() const { return query_; }
    void SetReceiveTime(TimePoint t) { receiveTime_ = t; }
    TimePoint GetReceiveTime() const { return receiveTime_; }
    void AddHeader(const char *start, const char *colon, const char *end);
    std::string GetHeader(const std::string &field) const;
    const std::map<std::string, std::string> &GetHeaders() const { return headers_; }

private:
    Method method_;
    Version version_;
    std::string path_;
    std::string query_;
    TimePoint receiveTime_;
    std::map<std::string, std::string> headers_;
};
#endif