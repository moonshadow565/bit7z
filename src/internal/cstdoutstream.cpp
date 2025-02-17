// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/cstdoutstream.hpp"

#include "internal/streamutil.hpp"

#include <iterator>

using namespace bit7z;

CStdOutStream::CStdOutStream( std::ostream& outputStream ) : mOutputStream( outputStream ) {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CStdOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( size == 0 ) {
        return S_OK;
    }

    const auto old_pos = mOutputStream.tellp();

    mOutputStream.write( static_cast< const char* >( data ), size );

    if ( processedSize != nullptr ) {
        *processedSize = static_cast< uint32_t >( mOutputStream.tellp() - old_pos );
    }

    return mOutputStream.bad() ? HRESULT_FROM_WIN32( ERROR_WRITE_FAULT ) : S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CStdOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) {
    std::ios_base::seekdir way; // NOLINT(cppcoreguidelines-init-variables)
    RINOK( to_seekdir( seekOrigin, way ) )

    /*if ( offset < 0 ) { //Tar sometimes uses negative offsets
        return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    }*/

    mOutputStream.seekp( static_cast< std::ostream::off_type >( offset ), way );

    if ( mOutputStream.bad() ) {
        return HRESULT_FROM_WIN32( ERROR_SEEK );
    }

    if ( newPosition != nullptr ) {
        *newPosition = static_cast< uint64_t >( mOutputStream.tellp() );
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CStdOutStream::SetSize( UInt64 newSize ) {
    if ( !mOutputStream ) {
        return E_FAIL;
    }

    const auto old_pos = mOutputStream.tellp();
    mOutputStream.seekp( 0, ostream::end );

    if ( !mOutputStream ) {
        return E_FAIL;
    }

    const auto current_pos = static_cast< uint64_t >( mOutputStream.tellp() );
    if ( newSize < current_pos ) {
        return E_FAIL;
    }

    const auto diff_pos = newSize - current_pos;
    if ( diff_pos > 0 ) {
        std::fill_n( std::ostream_iterator< char >( mOutputStream ), diff_pos, '\0' );
    }

    mOutputStream.seekp( old_pos );

    return mOutputStream ? S_OK : E_FAIL;
}

