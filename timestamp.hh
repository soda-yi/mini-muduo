#ifndef TIMESTAMP_HH
#define TIMESTAMP_HH

#include <string>
#include <sys/types.h>

class Timestamp
{
public:
    Timestamp()
        : microSecondsSinceEpoch_(0)
    {
    }

    explicit Timestamp(int64_t microSecondsSinceEpoch)
        : microSecondsSinceEpoch_(microSecondsSinceEpoch)
    {
    }

    std::string ToString() const;

    bool IsValid() const { return microSecondsSinceEpoch_ > 0; };
    int64_t GetMicroSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

    static Timestamp Now();
    static Timestamp Invalid() { return Timestamp(); }

    static constexpr int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.GetMicroSecondsSinceEpoch() < rhs.GetMicroSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.GetMicroSecondsSinceEpoch() == rhs.GetMicroSecondsSinceEpoch();
}

inline Timestamp AddTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.GetMicroSecondsSinceEpoch() + delta);
}

#endif