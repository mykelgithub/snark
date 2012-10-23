// This file is part of snark, a generic and flexible library
// for robotics research.
//
// Copyright (C) 2011 The University of Sydney
//
// snark is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// snark is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with snark. If not, see <http://www.gnu.org/licenses/>.

/// @author Cedric Wohlleber

#ifndef SNARK_MATH_INTERVAL_H_
#define SNARK_MATH_INTERVAL_H_

#include <iostream>
#include <comma/base/exception.h>
#include <comma/math/compare.h>
#include <Eigen/Core>

namespace snark { namespace math {

/// multi-dimensional interval
template < typename T, int N >
class interval
{
    public:
        typedef Eigen::Matrix< T, N, 1 > vector_type;

        /// constructor
        interval(): m_interval( std::make_pair( vector_type::Zero(), vector_type::Zero() ) ) {}
        
        /// constructor
        interval( const vector_type& min, const vector_type& max ) : m_interval( get_min( min, max ), get_max( min, max ) ) { if( less( max, min ) ) { COMMA_THROW( comma::exception, "invalid interval" ); } }

        /// constructor
        interval( const vector_type& min ) : m_interval( min, min ) { }

        /// return value
        const std::pair< vector_type, vector_type >& operator()() const { return m_interval; }

        /// return left boundary (convenience method)
        const vector_type& min() const { return m_interval.first; }

        /// return right boundary (convenience method)
        const vector_type& max() const { return m_interval.second; }

        /// return true, if variable belongs to the interval
        bool contains( const vector_type& t ) const { return ( ( m_interval.first.isApprox( t ) || less( m_interval.first, t ) ) && ( ( m_interval.second.isApprox( t ) || less( t, m_interval.second ) ) ) ); }

        /// return true, if variable belongs to the interval
        bool contains( const interval& rhs ) const { return !( less( rhs().first, m_interval.first ) || less( m_interval.second, rhs().second ) ); }

        /// compute the hull of the interval and [x]
        interval< T, N > hull( const vector_type& x ) { return interval( get_min( m_interval.first, x ), get_max( m_interval.second, x ) ); }

        /// compute the hull of 2 intervals
        interval< T, N > hull( const interval& rhs ) { return interval( get_min( m_interval.first, rhs.min() ), get_max( m_interval.second, rhs.max() ) ); }

        /// equality
        bool operator==( const interval& rhs ) const { return m_interval.first.isApprox( rhs().first ) && m_interval.second.isApprox( rhs().second ); }

        /// unequality
        bool operator!=( const interval& rhs ) const { return !operator==( rhs ); }

    private:
        static bool less( const vector_type& lhs, const vector_type& rhs ) { return ( ( lhs.array() < rhs.array() ).all() ); }
        static vector_type get_min( const vector_type& lhs, const vector_type& rhs ) { return rhs.array().min( lhs.array() ); }
        static vector_type get_max( const vector_type& lhs, const vector_type& rhs ) { return rhs.array().max( lhs.array() ); }
        
        std::pair< vector_type, vector_type > m_interval;
};

} } // namespace snark { namespace math {

#endif // SNARK_MATH_INTERVAL_H_