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
 * flexibility.
 */
#ifndef RANKED_SET_HH
#define RANKED_SET_HH
#include <iterator>
#include <ostream>
#include <set>
#include <string>
#include <utility>

template<class Value_type>
class Ranked_set
{
public:
  Ranked_set() = default;
  Ranked_set( int rank, const std::set<Value_type>& set ) : rank_( rank ), set_( set ) {}

  Ranked_set( const Ranked_set& other ) = default;
  Ranked_set( Ranked_set&& other ) noexcept = default;
  Ranked_set& operator=( Ranked_set&& other ) noexcept = default;
  Ranked_set& operator=( const Ranked_set& rhs ) = default;
  ~Ranked_set() = default;

  std::size_t size() const { return set_.size(); }

  int rank() const { return rank_; }

  void insert( const Value_type& elem ) { set_.insert( elem ); }

  void insert( const int rank, const Value_type& elem )
  {
    if ( set_.insert( elem ).second ) {
      rank_ += rank;
    }
  }

  void erase( const Value_type& elem ) { set_.erase( elem ); }

  void erase( const int rank, const Value_type& elem )
  {
    if ( set_.erase( elem ) ) {
      rank_ -= rank;
    }
  }

  void add( const int rankChange ) { rank_ += rankChange; }

  void subtract( const int rankChange ) { rank_ -= rankChange; }

  using container = typename std::set<Value_type>;
  using iterator = typename container::iterator;
  using const_iterator = typename container::const_iterator;

  iterator begin() const { return set_.begin(); }

  const_iterator cbegin() const { return set_.cbegin(); }

  iterator end() const { return set_.end(); }

  const_iterator cend() const { return set_.cend(); }

  friend std::ostream& operator<<( std::ostream& out, const Ranked_set<Value_type>& rs )
  {
    out << "{" << rs.rank_ << ",{";
    for ( const auto& s : rs.set_ ) {
      out << "\"" << s << "\",";
    }
    out << "}}" << std::endl;
    return out;
  }

  bool operator<( const Ranked_set& rhs ) const
  {
    return rhs.rank_ == rank_ ? this->set_ < rhs.set_ : this->rank_ < rhs.rank_;
  }
  explicit operator bool() const { return this->rank_ != 0 || this->cover_.size() != 0; }
  bool operator==( const Ranked_set<Value_type>& rhs ) const
  {
    return this->rank_ == rhs.rank_ && this->set_ == rhs.set_;
  }
  bool operator>( const Ranked_set<Value_type>& rhs ) const { return rhs < *this; }
  bool operator>=( const Ranked_set<Value_type>& rhs ) const { return !( *this < rhs ); }
  bool operator<=( const Ranked_set<Value_type>& rhs ) const { return !( *this > rhs ); }
  bool operator!=( const Ranked_set<Value_type>& rhs ) const { return !( *this == rhs ); }

private:
  int rank_ { 0 };
  std::set<Value_type> set_ {};
};

#endif // RANKED_SET_HH
