#ifndef SNARK_PERCEPTION_VOXELGRID_HEADER_GUARD
#define SNARK_PERCEPTION_VOXELGRID_HEADER_GUARD

#include <cmath>
#include <map>
#include <boost/none_t.hpp>
#include <Eigen/Core>
#include <boost/optional.hpp>
#include <comma/math/compare.h>
#include <snark/point_cloud/impl/pin_screen.h>

namespace snark {

/// voxel grid
/// @todo this class is mostly copy-pasted
///       just to allow refactoring elsewhere
///       refactor this class further, if needed
template < typename V = boost::none_t, typename P = Eigen::Vector3d >
class voxel_grid : public pin_screen< V >
{
    public:
        typedef V voxel_type;
        typedef P point_type;
        typedef typename pin_screen< V >::index_type index_type;
        typedef typename pin_screen< V >::size_type size_type;
        typedef typename pin_screen< V >::column_type column_type;
        typedef std::pair< P, P > extents_type;
        
        /// constructor
        /// @note if adjusted, the actual extents will be
        ///       extents - resolution / 2, extents + resolution / 2
        /// @todo get rid of "adjusted", it's just ugly
        voxel_grid( const extents_type& extents
                  , const point_type& resolution
                  , bool adjusted = false );

        /// return extents
        const extents_type& extents() const;

        /// return resolution
        const point_type& resolution() const;

        /// return index of the voxel covering a given datapoint
        index_type index_of( const point_type& p ) const;

        /// return true, if voxel grid covers a datapoint
        bool covers( const point_type& p ) const;

        /// create voxel that would cover given point and return index, if point is not out of bound
        voxel_type* touch_at( const point_type& d );

        /// erase voxel covering given point
        void erase_at( const point_type& p );

        /// return voxel origin
        point_type origin( const index_type& i ) const;

        /// return voxel origin
        point_type origin_at( const point_type& p ) const;

        /// return column of voxels
        const column_type* column( const point_type& p ) const;
        //column_type* column( const point_type& p ); // no non-const class in pin_screen for now

        using typename pin_screen< voxel_type >::iterator;
        using typename pin_screen< voxel_type >::const_iterator;
        using typename pin_screen< voxel_type >::neighbourhood_iterator;
        using pin_screen< voxel_type >::column;

    private:
        extents_type extents_;
        point_type resolution_;
};

namespace detail {

template < typename P >
inline static std::pair< P, P > extents( const std::pair< P, P >& e, const P& r, bool adjusted = true )
{
    return adjusted ? std::pair< P, P >( e.first - r / 2, e.second + r / 2 ) : e;
}

template < typename P >
inline static Eigen::Matrix< std::size_t, 1, 2 > size( const std::pair< P, P >& e, const P& r, bool adjusted = true )
{
    std::pair< P, P > f = extents( e, r, adjusted );
    P diff = f.second - f.first;
    return Eigen::Matrix< std::size_t, 1, 2 >( std::ceil( diff.x() / r.x() ), std::ceil( diff.y() / r.y() ) );
}

} // namespace detail {

template < typename V, typename P >
inline voxel_grid< V, P >::voxel_grid( const typename voxel_grid< V, P >::extents_type& extents
                                , const typename voxel_grid< V, P >::point_type& resolution
                                , bool adjusted )
    : pin_screen< V >( detail::size( extents, resolution, adjusted ) )
    , extents_( detail::extents( extents, resolution, adjusted ) )
    , resolution_( resolution )
{
}

template < typename V, typename P >
inline const std::pair< P, P >& voxel_grid< V, P >::extents() const { return extents_; }

template < typename V, typename P >
inline const P& voxel_grid< V, P >::resolution() const { return resolution_; }

template < typename V, typename P >
inline typename voxel_grid< V, P >::index_type voxel_grid< V, P >::index_of( const P& p ) const
{
    return index_type( std::floor( ( p.x() - extents_.first.x() ) / resolution_.x() )
                     , std::floor( ( p.y() - extents_.first.y() ) / resolution_.y() )
                     , std::floor( ( p.z() - extents_.first.z() ) / resolution_.z() ) );
}

template < typename V, typename P >
inline bool voxel_grid< V, P >::covers( const P& p ) const
{
    for( int i = 0; i < extents_.first.rows(); ++i ) // // quick and dirty: use traits, if decoupling from eigen needed
    {
        if( comma::math::less( p.array()[i], extents_.first.array()[i] ) ) { return false; }
        if( comma::math::less( extents_.second.array()[i], p.array()[i] ) ) { return false; }
    }
    return true;
}

template < typename V, typename P >
inline V* voxel_grid< V, P >::touch_at( const P& p )
{
    if( !covers( p ) ) { return NULL; }
    const index_type& i = index_of( p );
    return &pin_screen< V >::touch( i );
}

template < typename V, typename P >
inline void voxel_grid< V, P >::erase_at( const P& point )
{
    if( !covers( point ) ) { return; }
    pin_screen< V >::erase( index_of( point ) );
}

template < typename V, typename P >
inline P voxel_grid< V, P >::origin( const index_type& i ) const
{
    P p( resolution_[0] * i[0], resolution_[1] * i[1], resolution_[2] * i[2] );
    return extents_.first + p;
}

template < typename V, typename P >
inline P voxel_grid< V, P >::origin_at( const point_type& p ) const
{
    return origin( index_of( p ) );
}

template < typename V, typename P >
const typename voxel_grid< V, P >::column_type* voxel_grid< V, P >::column( const point_type& p ) const
{
    if( !covers( p ) ) { return NULL; }
    index_type index = index_of( p );
    return &this->pin_screen< V >::column( index.x(), index.y() );
}

// template < typename V, typename P >
// typename voxel_grid< V, P >::column_type* voxel_grid< V, P >::column( const point_type& p )
// {
//     if( !covers( p ) ) { return NULL; }
//     index i = index_of( p );
//     return &( this->column( i[0], i[1] ) );
// }

} // namespace snark {

#endif // SNARK_PERCEPTION_VOXELGRID_HEADER_GUARD