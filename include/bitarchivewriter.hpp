/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#ifndef BITARCHIVEWRITER_HPP
#define BITARCHIVEWRITER_HPP

#include "bitoutputarchive.hpp"

namespace bit7z {
    class BitArchiveWriter : public BitAbstractArchiveCreator, public BitOutputArchive {
        public:
            BitArchiveWriter( const Bit7zLibrary& lib, const BitInOutFormat& format );

            BitArchiveWriter( const Bit7zLibrary& lib,
                              const tstring& in_file,
                              const BitInOutFormat& format,
                              const tstring& password = TSTRING( "" ) );

            BitArchiveWriter( const Bit7zLibrary& lib,
                              const vector< byte_t >& in_buffer,
                              const BitInOutFormat& format,
                              const tstring& password = TSTRING( "" ) );

            BitArchiveWriter( const Bit7zLibrary& lib,
                              std::istream& in_stream,
                              const BitInOutFormat& format,
                              const tstring& password = TSTRING( "" ) );
    };
}

#endif //BITARCHIVEWRITER_HPP
