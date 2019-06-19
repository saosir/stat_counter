#ifndef stat_counter_h__
#define stat_counter_h__
#include <time.h>
#include <deque>
#include <numeric>
#include <algorithm>

class BaseCounter {
public:
    BaseCounter(){};
    virtual ~BaseCounter(){};
    virtual int event(int v) = 0;
    virtual void value(int v) = 0;
    virtual int sum() = 0;
    virtual double avg() = 0;
    virtual int empty() = 0;
};


class TotalCounter : public BaseCounter {
public:
    TotalCounter()
        : count_(0)
    {
    }

    int event(int v)
    {
        count_ += v;
        return 0;
    }

    void value(int v)
    {
        count_ = v;
    }

    int sum()
    {
        return count_;
    }

    double avg()
    {
        return count_;
    }

    int empty()
    {
        return count_ == 0;
    }

protected:
    int count_;
};


class AverageWindowCounter : public BaseCounter {
public:
    AverageWindowCounter(size_t window_size = 300)
        : window_size_(window_size)
    {
    }

    int event(int v)
    {
        values_.push_back(v);
        if (values_.size() > window_size_) {
            values_.pop_front();
        }

        return 0;
    }

    void value(int v)
    {
        event(v);
    }

    int sum()
    {
        return std::accumulate(values_.begin(), values_.end(), 0);
    }

    double avg()
    {
        return 1.0 * this->sum() / values_.size();
    }

    int empty()
    {
        return values_.size() == 0;
    }

protected:
    size_t window_size_;
    std::deque<int> values_;
};


class TimeBaseAverageEventCounter : public BaseCounter {
public:
    TimeBaseAverageEventCounter(int window_size = 300, int window_internal = 10)
        : window_size_(window_size)
        , window_interval_(window_internal)
        , cache_value_(0)
        , cache_start_(0)
        , first_data_time_(0)
    {
        max_window_size_ = window_size_;
    }

    int event(int v)
    {
        int now = time(NULL);
        if (first_data_time_) {
            first_data_time_ = now;
        }

        if (cache_start_ == 0) {
            cache_value_ = v;
            cache_event_ = 1;
            cache_start_ = now;
        }
        else if (now - cache_start_ >= window_interval_) {
            values_.push_back(cache_value_);
            events_.push_back(cache_event_);
            times_.push_back(cache_start_);
            on_append(cache_value_, cache_event_);
            cache_value_ = v;
            cache_event_ = 1;
            cache_start_ = now;
        }
        else {
            cache_value_ += v;
            cache_event_ += 1;
        }
        return 0;
    }
    virtual void on_append(int value, int event) {}

    void value(int v)
    {
        cache_value_ = v;
    }

    int sum()
    {
        _trim_window();
        return std::accumulate(values_.begin(), values_.end(), 0) + cache_value_;
    }

    double avg()
    {
        int events = std::accumulate(events_.begin(), events_.end(), 0) + cache_event_;
        if (events == 0)
            return 0;
        return 1.0 * sum() / events;
    }

    int empty()
    {
        _trim_window();
        return values_.empty() && cache_start_ == 0;
    }

private:
    void _trim_window()
    {
        int now = time(NULL);
        if (cache_start_ > 0 && now - cache_start_ >= window_interval_) {
            values_.push_back(cache_value_);
            events_.push_back(cache_event_);
            times_.push_back(cache_start_);
            on_append(cache_value_, cache_event_);
            cache_value_ = 0;
            cache_event_ = 0;
            cache_start_ = 0;
        }

        if (window_size_ != max_window_size_ && first_data_time_ > 0) {
            int passed_window_size = (now - first_data_time_) / window_interval_;
            window_size_ = max_window_size_ < passed_window_size ? max_window_size_ : passed_window_size;
        }
        int window_limit_time = now - window_size_ * window_interval_;
        while (times_.size() > 0 && times_[0] < window_limit_time) {
            times_.pop_front();
            values_.pop_front();
            events_.pop_front();
        }
    }
    std::deque<int> values_;
    std::deque<int> events_;
    std::deque<int> times_;
    int window_size_;
    int window_interval_;
    int max_window_size_;
    int cache_value_;
    int cache_event_;
    int cache_start_;
    int first_data_time_;
};


class TimeBaseAverageWindowCounter : public BaseCounter {
public:
    TimeBaseAverageWindowCounter(int window_size = 300, int window_interval = 10)
        : window_size_(window_size)
        , window_interval_(window_interval)
        , cache_value_(0)
        , cache_start_(0)
        , first_data_time_(0)
    {
        max_window_size_ = window_size_;
    }

    ~TimeBaseAverageWindowCounter() {}

    int event(int v)
    {
        int now = time(NULL);
        if (first_data_time_ == 0)
            first_data_time_ = now;

        if (cache_start_ == 0) {
            cache_value_ = v;
            cache_start_ = now;
        }
        else if (now - cache_start_ >= window_interval_) {
            values_.push_back(v);
            times_.push_back(now);
            on_append(v, now);
            cache_value_ = v;
            cache_start_ = now;
        }
        else
            cache_value_ += v;

        return 0;
    }

    void value(int v)
    {
        cache_value_ = v;
    }

    int sum()
    {
        _trim_window();
        return std::accumulate(values_.begin(), values_.end(), 0) + cache_value_;
    }

    double avg()
    {
        int s = sum();
        if (window_size_ <= 0 || window_interval_ <= 0)
            return 0;
        return 1.0 * s / window_size_ / window_interval_;
    }

    int empty()
    {
        _trim_window();
        return values_.empty() && cache_start_ == 0;
    }

    virtual void on_append(int v, int t) {}

private:
    void _trim_window()
    {
        int now = time(NULL);
        if (cache_start_ > 0 && now - cache_start_ >= window_interval_) {
            values_.push_back(cache_value_);
            times_.push_back(cache_start_);
            on_append(cache_value_, cache_start_);
            cache_value_ = 0;
            cache_start_ = 0;
        }

        if (window_size_ != max_window_size_ && first_data_time_ > 0) {
            int passed_window_size = (now - first_data_time_) / window_interval_;
            window_size_ = max_window_size_ < passed_window_size ? max_window_size_ : passed_window_size;
        }
        int window_limit_time = now - window_size_ * window_interval_;
        while (times_.size() > 0 && times_[0] < window_limit_time) {
            times_.pop_front();
            values_.pop_front();
        }
    }

protected:
    int window_size_;
    int window_interval_;
    int max_window_size_;
    int cache_value_;
    int cache_start_;
    int first_data_time_;
    std::deque<int> values_;
    std::deque<int> times_;
};

#endif // stat_counter_h__
