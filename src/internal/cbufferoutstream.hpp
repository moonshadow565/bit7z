/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CBUFFEROUTSTREAM_HPP
#define CBUFFEROUTSTREAM_HPP

#include "bittypes.hpp"
#include "internal/guids.hpp"
#include "internal/macros.hpp"

#include <7zip/IStream.h>
#include <Common/MyCom.h>

namespace bit7z {

using std::vector;

class CBufferOutStream final : public IOutStream, public CMyUnknownImp {
    public:
        explicit CBufferOutStream( vector< byte_t >& out_buffer );

        CBufferOutStream( const CBufferOutStream& ) = delete;

        CBufferOutStream( CBufferOutStream&& ) = delete;

        CBufferOutStream& operator=( const CBufferOutStream& ) = delete;

        CBufferOutStream& operator=( CBufferOutStream&& ) = delete;

        MY_UNKNOWN_DESTRUCTOR( ~CBufferOutStream() ) = default;

        MY_UNKNOWN_IMP1( IOutStream ) // NOLINT(modernize-use-noexcept)

        // IOutStream
        BIT7Z_STDMETHOD( Write, const void* data, UInt32 size, UInt32* processedSize );

        BIT7Z_STDMETHOD_NOEXCEPT( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

        BIT7Z_STDMETHOD( SetSize, UInt64 newSize );

    private:
        buffer_t& mBuffer;
        buffer_t::iterator mCurrentPosition;
};

}  // namespace bit7z

#endif // CBUFFEROUTSTREAM_HPP
