// This file is part of snark, a generic and flexible library for robotics research
// Copyright (c) 2011 The University of Sydney
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the The University of Sydney.
// 4. Neither the name of the The University of Sydney nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


/// @author Cedric Wohlleber

#ifndef SNARK_GRAPHICS_APPLICATIONS_VIEWPOINTS_VERTEX_BUFFER_H_
#define SNARK_GRAPHICS_APPLICATIONS_VIEWPOINTS_VERTEX_BUFFER_H_

#include <Qt3D/qvector3darray.h>
#include <Qt3D/qcolor4ub.h>
#include "./block_buffer.h"

namespace snark { namespace graphics { namespace qt3d {

/// circular double buffer for vertices and color
/// accumulating points blockwise
/// once the block is complete
///
/// @todo terribly coupled; use generic vertex_buffer implementation in view-points
///       it is easy to refactor, just a bit fiddly, since it is used in label-points, too
class vertex_buffer
{
    public:
        /// constructor
        vertex_buffer( std::size_t size );

        /// add vertex
        /// @param point vertex coordinates
        /// @param color vertex color
        /// @param block id of a block of vertices; on change of block id, double buffer toggles
        void add_vertex( const QVector3D& point, const QColor4ub& color, unsigned int block = 0 );

        /// toggle double buffer, so that the buffer currently accumulating (the "write" buffer)
        /// becomes available for reading
        ///
        /// adding vertices after this call may produce strange results
        ///
        /// @note should be done on the last block, since otherwise
        ///       there is no way to determine that the last block has
        ///       been finished and thus the last block will not be
        ///       properly visualized
        void toggle();

        /// return points buffer
        const QVector3DArray& points() const;

        /// return colour buffer
        const QArray<QColor4ub>& color() const;

        /// return current size of the buffer that is ready for reading
        unsigned int size() const;

        /// return current offset in the buffer that is ready for reading
        unsigned int index() const;

    protected:
        unsigned int read_index_;
        unsigned int write_index_;
        unsigned int read_size_;
        unsigned int write_end_;
        unsigned int write_size_;
        unsigned int buffer_size_;
        unsigned int block_;
        QVector3DArray points_;
        QArray<QColor4ub> color_;
};

} } } // namespace snark { namespace graphics { namespace qt3d {

#endif /*SNARK_GRAPHICS_APPLICATIONS_VIEWPOINTS_VERTEX_BUFFER_H_*/
