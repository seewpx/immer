//
// immer - immutable data structures for C++
// Copyright (C) 2016, 2017 Juan Pedro Bolivar Puente
//
// This file is part of immer.
//
// immer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// immer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with immer.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include <immer/memory_policy.hpp>
#include <immer/detail/hamts/champ.hpp>
#include <immer/detail/hamts/champ_iterator.hpp>

#include <functional>

namespace immer {

template <typename T,
          typename Hash,
          typename Equal,
          typename MemoryPolicy,
          detail::hamts::bits_t B>
class set_transient;

/*!
 * Immutable set representing an unordered bag of values.
 *
 * @tparam T    The type of the values to be stored in the container.
 * @tparam Hash The type of a function object capable of hashing
 *              values of type `T`.
 * @tparam Equal The type of a function object capable of comparing
 *              values of type `T`.
 * @tparam MemoryPolicy Memory management policy. See @ref
 *              memory_policy.
 *
 * @rst
 *
 * This cotainer provides a good trade-off between cache locality,
 * membership checks, update performance and structural sharing.  It
 * does so by storing the data in contiguous chunks of :math:`2^{B}`
 * elements.  When storing big objects, the size of these contiguous
 * chunks can become too big, damaging performance.  If this is
 * measured to be problematic for a specific use-case, it can be
 * solved by using a `immer::box` to wrap the type `T`.
 *
 * **Example**
 *   .. literalinclude:: ../example/set/intro.cpp
 *      :language: c++
 *      :start-after: intro/start
 *      :end-before:  intro/end
 *
 * @endrst
 *
 */
template <typename T,
          typename Hash          = std::hash<T>,
          typename Equal         = std::equal_to<T>,
          typename MemoryPolicy  = default_memory_policy,
          detail::hamts::bits_t B = default_bits>
class set
{
    using impl_t = detail::hamts::champ<T, Hash, Equal, MemoryPolicy, B>;

public:
    using key_type = T;
    using value_type = T;
    using size_type = detail::hamts::size_t;
    using diference_type = std::ptrdiff_t;
    using hasher = Hash;
    using key_equal = Equal;
    using reference = const T&;
    using const_reference = const T&;

    using iterator         = detail::hamts::champ_iterator<T, Hash, Equal,
                                                         MemoryPolicy, B>;
    using const_iterator   = iterator;

    using transient_type   = set_transient<T, Hash, Equal, MemoryPolicy, B>;

    /*!
     * Default constructor.  It creates a set of `size() == 0`.  It
     * does not allocate memory and its complexity is @f$ O(1) @f$.
     */
    set() = default;

    /*!
     * Returns an iterator pointing at the first element of the
     * collection. It does not allocate memory and its complexity is
     * @f$ O(1) @f$.
     */
    iterator begin() const { return {impl_}; }

    /*!
     * Returns an iterator pointing just after the last element of the
     * collection. It does not allocate and its complexity is @f$ O(1) @f$.
     */
    iterator end() const { return {impl_, typename iterator::end_t{}}; }

    /*!
     * Returns the number of elements in the container.  It does
     * not allocate memory and its complexity is @f$ O(1) @f$.
     */
    size_type size() const { return impl_.size; }

    /*!
     * Returns `1` when `value` is contained in the set or `0`
     * otherwise. It won't allocate memory and its complexity is
     * *effectively* @f$ O(1) @f$.
     */
    size_type count(const T& value) const
    { return impl_.template get<detail::constantly<size_type, 1>,
                                detail::constantly<size_type, 0>>(value); }

    /*!
     * Returns whether the sets are equal.
     */
    bool operator==(const set& other) const
    { return impl_.equals(other.impl_); }
    bool operator!=(const set& other) const
    { return !(*this == other); }

    /*!
     * Returns a set containing `value`.  If the `value` is already in
     * the set, it returns the same set.  It may allocate memory and
     * its complexity is *effectively* @f$ O(1) @f$.
     *
     * @rst
     *
     * **Example**
     *   .. literalinclude:: ../example/set/set.cpp
     *      :language: c++
     *      :dedent: 8
     *      :start-after: insert/start
     *      :end-before:  insert/end
     *
     * @endrst
     */
    set insert(T value) const
    { return impl_.add(std::move(value)); }

    /*!
     * Returns a set without `value`.  If the `value` is not in the
     * set it returns the same set.  It may allocate memory and its
     * complexity is *effectively* @f$ O(1) @f$.
     *
     * @rst
     *
     * **Example**
     *   .. literalinclude:: ../example/set/set.cpp
     *      :language: c++
     *      :dedent: 8
     *      :start-after: erase/start
     *      :end-before:  erase/end
     *
     * @endrst
     */
    set erase(const T& value) const
    { return impl_.sub(value); }

    /*!
     * Returns an @a transient form of this container, a
     * `immer::set_transient`.
     */
    transient_type transient() const&
    { return transient_type{ impl_ }; }
    transient_type transient() &&
    { return transient_type{ std::move(impl_) }; }

    // Semi-private
    const impl_t& impl() const { return impl_; }

private:
    friend transient_type;

    set(impl_t impl)
        : impl_(std::move(impl))
    {}

    impl_t impl_ = impl_t::empty;
};

} // namespace immer
