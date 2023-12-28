/**
 * MIT License
 *
 * Copyright (c) 2023 Alex G. Lopez
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author: Alexander Lopez
 * File: Ranked_set.h
 * -----------------------
 * This template class is a simple wrapper for a set so that I can implement my scoring method for
 * the exact and overlapping covers I find for Pokemon. However, this could be used for any case
 * in which I need a set to carry a numeric rank that defines its natural order. The order goes as
 * follows: Ranked_sets are ordered first by their numeric rank in ascending order. If two ranks
 * are the same the Ranked_set is then organized by the natural ordering of std::set and behaves
 * exactly as you would expect the C++ std::set to behave when compared. The rank component of
 * the set is a simple integer, allowing for both positive and negative weights.
 *
 * Initially, my purpose in using this was to create the same data structure as a priority queue.
 * I found that the C++ priority queue was inconvenient because iteration through a priority queue
 * of Sets required me to pop from the queue, destroying it. This gives me less flexibility with
 * how I store and illustrate solutions to cover problems in the Pokemon GUI. Putting Ranked_sets
 * in a set acheives the same ordering as I wanted in a priority queue and allows for more
 * flexibility. Becuase the Ranked set sizes we deal with are very small in my use case I found a flat set
 * to be the better implementation for locality and convenience.
 */
#pragma once
#ifndef RANKED_SET_HH
#define RANKED_SET_HH
#include <algorithm>
#include <compare>
#include <cstddef>
#include <ostream>
#include <utility>
#include <vector>

/// Implemented as a flat set meaning you should reserve known sizes ahead of time when possible.
template<class T>
class Ranked_set
{
public:
  Ranked_set() = default;
  Ranked_set( int rank, std::vector<T>&& set ) : rank_( rank ), flat_set_( std::move( set ) )
  {
    std::sort( flat_set_.begin(), flat_set_.end() );
  }

  Ranked_set( const Ranked_set& other ) = default;
  Ranked_set( Ranked_set&& other ) noexcept = default;
  Ranked_set& operator=( Ranked_set&& other ) noexcept = default;
  Ranked_set& operator=( const Ranked_set& rhs ) = default;
  ~Ranked_set() = default;

  [[nodiscard]] std::size_t size() const
  {
    return flat_set_.size();
  }

  [[nodiscard]] std::size_t empty() const
  {
    return flat_set_.empty();
  }

  [[nodiscard]] int rank() const
  {
    return rank_;
  }

  /// Recommended to use if you know how many elements you will store at max. Makes flat set faster.
  void reserve( size_t size )
  {
    flat_set_.reserve( size );
  }

  [[nodiscard]] bool insert( const T& elem )
  {
    const auto found = std::lower_bound( flat_set_.cbegin(), flat_set_.cend(), elem );
    if ( found != flat_set_.end() && *found == elem ) {
      return false;
    }
    static_cast<void>( flat_set_.insert( found, elem ) );
    return true;
  }

  [[nodiscard]] bool insert( const int rank, const T& elem )
  {
    const auto found = std::lower_bound( flat_set_.cbegin(), flat_set_.cend(), elem );
    if ( found != flat_set_.end() && *found == elem ) {
      return false;
    }
    rank_ += rank;
    static_cast<void>( flat_set_.insert( found, elem ) );
    return true;
  }

  [[nodiscard]] bool erase( const T& elem )
  {
    auto found = std::lower_bound( flat_set_.cbegin(), flat_set_.cend(), elem );
    if ( found != flat_set_.end() && *found == elem ) {
      static_cast<void>( flat_set_.erase( found ) );
      return true;
    }
    return false;
  }

  [[nodiscard]] bool erase( const int rank, const T& elem )
  {
    auto found = std::lower_bound( flat_set_.cbegin(), flat_set_.cend(), elem );
    if ( found != flat_set_.end() && *found == elem ) {
      rank_ -= rank;
      static_cast<void>( flat_set_.erase( found ) );
      return true;
    }
    return false;
  }

  void add( const int rank_change )
  {
    rank_ += rank_change;
  }

  void subtract( const int rank_change )
  {
    rank_ -= rank_change;
  }

  using container = typename std::vector<T>;
  using iterator = typename container::iterator;
  using const_iterator = typename container::const_iterator;

  iterator begin()
  {
    return flat_set_.begin();
  }

  const_iterator cbegin() const
  {
    return flat_set_.cbegin();
  }

  iterator end()
  {
    return flat_set_.end();
  }

  const_iterator cend() const
  {
    return flat_set_.cend();
  }

  friend std::ostream& operator<<( std::ostream& out, const Ranked_set<T>& rs )
  {
    out << "{" << rs.rank_ << ",{";
    for ( const auto& s : rs.flat_set_ ) {
      out << "\"" << s << "\",";
    }
    out << "}}\n";
    return out;
  }

  explicit operator bool() const
  {
    return this->rank_ != 0 || this->cover_.size() != 0;
  }
  bool operator==( const Ranked_set<T>& rhs ) const
  {
    return this->rank_ == rhs.rank_ && this->flat_set_ == rhs.flat_set_;
  }
  std::weak_ordering operator<=>( const Ranked_set<T>& rhs ) const
  {
    return this->rank_ == rhs.rank_ ? this->flat_set_ <=> rhs.flat_set_ : this->rank_ <=> rhs.rank_;
  }

private:
  int rank_ { 0 };
  std::vector<T> flat_set_ {};
};

#endif // RANKED_SET_HH
