// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include "bitfilecompressor.hpp"

#include "biterror.hpp"
#include "bitexception.hpp"
#include "bitoutputarchive.hpp"
#include "internal/updatecallback.hpp" //for exception strings

using namespace std;
using namespace bit7z;

BitFileCompressor::BitFileCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitCompressor( lib, format ) {}

/* from filesystem to filesystem */

void BitFileCompressor::compress( const vector< tstring >& in_paths, const tstring& out_file ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive output_archive{ *this, out_file };
    output_archive.addItems( in_paths );
    output_archive.compressTo( out_file );
}

void BitFileCompressor::compress( const map< tstring, tstring >& in_paths, const tstring& out_file ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive output_archive{ *this, out_file };
    output_archive.addItems( in_paths );
    output_archive.compressTo( out_file );
}

void BitFileCompressor::compressFiles( const vector< tstring >& in_files, const tstring& out_file ) const {
    if ( in_files.size() > 1 && !mFormat.hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive output_archive{ *this, out_file };
    output_archive.addFiles( in_files );
    output_archive.compressTo( out_file );
}

void BitFileCompressor::compressFiles( const tstring& in_dir, const tstring& out_file,
                                       bool recursive, const tstring& filter ) const {
    if ( !mFormat.hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive output_archive{ *this, out_file };
    output_archive.addFiles( in_dir, recursive, filter );
    output_archive.compressTo( out_file );
}

void BitFileCompressor::compressDirectory( const tstring& in_dir, const tstring& out_file ) const {
    compressFiles( in_dir, out_file, true, TSTRING( "" ) );
}

/* from filesystem to stream */

void BitFileCompressor::compress( const vector< tstring >& in_paths, ostream& out_stream ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive output_archive{ *this };
    output_archive.addItems( in_paths );
    output_archive.compressTo( out_stream );
}

void BitFileCompressor::compress( const map< tstring, tstring >& in_paths, ostream& out_stream ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive output_archive{ *this };
    output_archive.addItems( in_paths );
    output_archive.compressTo( out_stream );
}
