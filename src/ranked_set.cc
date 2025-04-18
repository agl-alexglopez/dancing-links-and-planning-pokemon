module;
#include <algorithm>
#include <compare>
#include <cstddef>
#include <ostream>
#include <vector>
export module dancing_links:ranked_set;

/// Implemented as a flat set meaning you should reserve known sizes ahead of
/// time when possible.
export template <class T> class Ranked_set {
  public:
    Ranked_set() = default;
    Ranked_set(int rank, std::vector<T> &&set)
        : rank_(rank), flat_set_(std::move(set))
    {
        std::sort(flat_set_.begin(), flat_set_.end());
    }

    Ranked_set(const Ranked_set &other) = default;
    Ranked_set(Ranked_set &&other) noexcept = default;
    Ranked_set &operator=(Ranked_set &&other) noexcept = default;
    Ranked_set &operator=(const Ranked_set &rhs) = default;
    ~Ranked_set() = default;

    [[nodiscard]] std::size_t
    size() const
    {
        return flat_set_.size();
    }

    [[nodiscard]] std::size_t
    empty() const
    {
        return flat_set_.empty();
    }

    [[nodiscard]] int
    rank() const
    {
        return rank_;
    }

    /// Recommended to use if you know how many elements you will store at max.
    /// Makes flat set faster.
    void
    reserve(size_t size)
    {
        flat_set_.reserve(size);
    }

    [[nodiscard]] bool
    insert(const T &elem)
    {
        const auto found
            = std::lower_bound(flat_set_.cbegin(), flat_set_.cend(), elem);
        if (found != flat_set_.end() && *found == elem)
        {
            return false;
        }
        static_cast<void>(flat_set_.insert(found, elem));
        return true;
    }

    [[nodiscard]] bool
    insert(const int rank, const T &elem)
    {
        const auto found
            = std::lower_bound(flat_set_.cbegin(), flat_set_.cend(), elem);
        if (found != flat_set_.end() && *found == elem)
        {
            return false;
        }
        rank_ += rank;
        static_cast<void>(flat_set_.insert(found, elem));
        return true;
    }

    [[nodiscard]] bool
    erase(const T &elem)
    {
        auto found
            = std::lower_bound(flat_set_.cbegin(), flat_set_.cend(), elem);
        if (found != flat_set_.end() && *found == elem)
        {
            static_cast<void>(flat_set_.erase(found));
            return true;
        }
        return false;
    }

    [[nodiscard]] bool
    erase(const int rank, const T &elem)
    {
        auto found
            = std::lower_bound(flat_set_.cbegin(), flat_set_.cend(), elem);
        if (found != flat_set_.end() && *found == elem)
        {
            rank_ -= rank;
            static_cast<void>(flat_set_.erase(found));
            return true;
        }
        return false;
    }

    void
    add(const int rank_change)
    {
        rank_ += rank_change;
    }

    void
    subtract(const int rank_change)
    {
        rank_ -= rank_change;
    }

    using container = typename std::vector<T>;
    using iterator = typename container::iterator;
    using const_iterator = typename container::const_iterator;

    const_iterator
    begin() const
    {
        return flat_set_.cbegin();
    }

    const_iterator
    cbegin() const
    {
        return flat_set_.cbegin();
    }

    const_iterator
    end() const
    {
        return flat_set_.cend();
    }

    const_iterator
    cend() const
    {
        return flat_set_.cend();
    }

    friend std::ostream &
    operator<<(std::ostream &out, const Ranked_set<T> &rs)
    {
        out << "{" << rs.rank_ << ",{";
        for (const auto &s : rs.flat_set_)
        {
            out << "\"" << s << "\",";
        }
        out << "}}\n";
        return out;
    }

    explicit
    operator bool() const
    {
        return this->rank_ != 0 || !this->flat_set_.empty();
    }
    bool
    operator==(const Ranked_set<T> &rhs) const
    {
        return this->rank_ == rhs.rank_ && this->flat_set_ == rhs.flat_set_;
    }
    std::weak_ordering
    operator<=>(const Ranked_set<T> &rhs) const
    {
        return this->rank_ == rhs.rank_ ? this->flat_set_ <=> rhs.flat_set_
                                        : this->rank_ <=> rhs.rank_;
    }

  private:
    int rank_{0};
    std::vector<T> flat_set_{};
};
