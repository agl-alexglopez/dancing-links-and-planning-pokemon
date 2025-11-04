module;
#include <algorithm>
#include <compare>
#include <cstddef>
#include <initializer_list>
#include <ostream>
#include <vector>
export module dancing_links:ranked_set;

/// A Ranked_set implements an ordered set in a flat container. This means
/// the elements it stores are not pointer stable when any insertion, removal,
/// or resizing takes place. They are contiguous.
///
/// A ranked set holds any type and an integer rank the user can interact
/// with regarding the set of this type. The user is free to increase or
/// decrease the rank via member functions.
///
/// A Ranked_set is also optimized for quickly sifting through Dancing Links
/// generated graph cover solutions. The std::strong_ordering is defined as
/// follows.
///
///   1. First the integer ranks are compared via <=>.
///   2. If the ranks are equivalent, then the set sizes are compared. Because
///      we are ranking teams of Pokemon or number of attack slots we need
///      to use, a smaller set is more valuable as it gives more freedom for
///      other Pokemon in the team or attack slots to fill.
///   3. Finally, if the ranks and the set sizes are equivalent we fall back
///      to calling <=> for the underlying container which returns the
///      lexicographical ordering of the stored elements. This is a linear time
///      operation given the size of the smaller of the two containers.
///
/// Though this is a template class, it is specialized to the Dancing Links
/// and graph cover problem space and care should be taken if using it for
/// other purposes.
export template <class T> class Ranked_set {
  public:
    Ranked_set() = default;
    Ranked_set(int rank, std::initializer_list<T> set)
        : rank_(rank), flat_set_(std::move(set))
    {
        std::sort(flat_set_.begin(), flat_set_.end());
    }

    Ranked_set(Ranked_set const &other) = default;
    Ranked_set(Ranked_set &&other) noexcept = default;
    Ranked_set &operator=(Ranked_set &&other) noexcept = default;
    Ranked_set &operator=(Ranked_set const &rhs) = default;
    ~Ranked_set() = default;

    [[nodiscard]] constexpr std::size_t
    size() const noexcept
    {
        return flat_set_.size();
    }

    [[nodiscard]] constexpr bool
    empty() const noexcept
    {
        return flat_set_.empty();
    }

    [[nodiscard]] constexpr int
    rank() const noexcept
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
    insert(T const &elem)
    {
        auto const found
            = std::lower_bound(flat_set_.cbegin(), flat_set_.cend(), elem);
        if (found != flat_set_.end() && *found == elem)
        {
            return false;
        }
        static_cast<void>(flat_set_.insert(found, elem));
        return true;
    }

    [[nodiscard]] bool
    insert(int const rank, T const &elem)
    {
        auto const found
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
    erase(T const &elem)
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
    erase(int const rank, T const &elem)
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
    add(int const rank_change)
    {
        rank_ += rank_change;
    }

    void
    subtract(int const rank_change)
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
    cbegin() const noexcept
    {
        return flat_set_.cbegin();
    }

    const_iterator
    end() const
    {
        return flat_set_.cend();
    }

    const_iterator
    cend() const noexcept
    {
        return flat_set_.cend();
    }

    friend std::ostream &
    operator<<(std::ostream &out, Ranked_set<T> const &rs)
    {
        out << "{" << rs.rank_ << ",{";
        for (auto const &s : rs.flat_set_)
        {
            out << "\"" << s << "\",";
        }
        out << "}}\n";
        return out;
    }

    explicit
    operator bool() const noexcept
    {
        return rank_ != 0 || !flat_set_.empty();
    }

    constexpr bool
    operator==(Ranked_set<T> const &rhs) const
    {
        return rank_ == rhs.rank_ && flat_set_ == rhs.flat_set_;
    }

    constexpr std::strong_ordering
    operator<=>(Ranked_set<T> const &rhs) const
    {
        std::strong_ordering cmp = rank_ <=> rhs.rank_;
        if (std::strong_ordering::equal != cmp)
        {
            return cmp;
        }
        cmp = flat_set_.size() <=> rhs.flat_set_.size();
        if (std::strong_ordering::equal != cmp)
        {
            return cmp;
        }
        return flat_set_ <=> rhs.flat_set_;
    }

  private:
    int rank_{0};
    std::vector<T> flat_set_{};
};
