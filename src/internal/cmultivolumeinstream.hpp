/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CMULTIVOLUMEINSTREAM_HPP
#define CMULTIVOLUMEINSTREAM_HPP

#include "internal/cvolumeinstream.hpp"
#include "internal/macros.hpp"
#include "internal/guiddef.hpp"

#include <7zip/IStream.h>
#include <Common/MyCom.h>

namespace bit7z {

class CMultiVolumeInStream : public IInStream, public CMyUnknownImp {
        uint64_t mCurrentPosition;
        uint64_t mTotalSize;

        std::vector< CMyComPtr< CVolumeInStream > > mVolumes;

        const CMyComPtr< CVolumeInStream >& currentVolume();

        void addVolume( const fs::path& volume_path );

    public:
        explicit CMultiVolumeInStream( const fs::path& first_volume );

        CMultiVolumeInStream( const CMultiVolumeInStream& ) = delete;

        CMultiVolumeInStream( CMultiVolumeInStream&& ) = delete;

        CMultiVolumeInStream& operator=( const CMultiVolumeInStream& ) = delete;

        CMultiVolumeInStream& operator=( CMultiVolumeInStream&& ) = delete;

        MY_UNKNOWN_VIRTUAL_DESTRUCTOR( ~CMultiVolumeInStream() ) = default;

        MY_UNKNOWN_IMP1( IInStream )

        BIT7Z_STDMETHOD( Read, void* data, UInt32 size, UInt32* processedSize );

        BIT7Z_STDMETHOD( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );
};

} // namespace bit7z

#endif //CMULTIVOLUMEINSTREAM_HPP
