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

#ifdef BIT7Z_AUTO_FORMAT

#include <algorithm>
#include "internal/formatdetect.hpp"

#if defined(BIT7Z_USE_NATIVE_STRING) && defined(_WIN32)
#include <cwctype> // for std::iswdigit
#else
#include <cctype> // for std::isdigit
#endif

#include "biterror.hpp"
#include "bitexception.hpp"
#include "internal/fsutil.hpp"
#ifndef _WIN32
#include "internal/guiddef.hpp"
#endif

#include <7zip/IStream.h>

#if defined(_WIN32)
#define bswap64 _byteswap_uint64
#elif defined(__GNUC__) || defined(__clang__)
//Note: the versions of gcc and clang that can compile bit7z should also have this builtin, hence there is no need
//      for checking compiler version or using _has_builtin macro!
#define bswap64 __builtin_bswap64
#else
static inline uint64_t bswap64 (uint64_t x) {
    return  ((x << 56) & 0xff00000000000000ULL) |
            ((x << 40) & 0x00ff000000000000ULL) |
            ((x << 24) & 0x0000ff0000000000ULL) |
            ((x << 8)  & 0x000000ff00000000ULL) |
            ((x >> 8)  & 0x00000000ff000000ULL) |
            ((x >> 24) & 0x0000000000ff0000ULL) |
            ((x >> 40) & 0x000000000000ff00ULL) |
            ((x >> 56) & 0x00000000000000ffULL);
}
#endif

uint64_t constexpr str_hash( bit7z::tchar const* input ) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return *input != 0 ? static_cast< uint64_t >( *input ) + 33 * str_hash( input + 1 ) : 5381;
}

namespace bit7z {
/* NOTE: Until v3, a std::unordered_map was used for mapping the extensions and the corresponding
 *       format, however the ifs are faster and have less memory footprint. */
bool findFormatByExtension( const tstring& ext, const BitInFormat** format ) {
    switch ( str_hash( ext.c_str() ) ) {
        case str_hash( BIT7Z_STRING( "7z" ) ):
            *format = &BitFormat::SevenZip;
            return true;
        case str_hash( BIT7Z_STRING( "bzip2" ) ):
        case str_hash( BIT7Z_STRING( "bz2" ) ):
        case str_hash( BIT7Z_STRING( "tbz2" ) ):
        case str_hash( BIT7Z_STRING( "tbz" ) ):
            *format = &BitFormat::BZip2;
            return true;
        case str_hash( BIT7Z_STRING( "gz" ) ):
        case str_hash( BIT7Z_STRING( "gzip" ) ):
        case str_hash( BIT7Z_STRING( "tgz" ) ):
            *format = &BitFormat::GZip;
            return true;
        case str_hash( BIT7Z_STRING( "tar" ) ):
            *format = &BitFormat::Tar;
            return true;
        case str_hash( BIT7Z_STRING( "wim" ) ):
        case str_hash( BIT7Z_STRING( "swm" ) ):
            *format = &BitFormat::Wim;
            return true;
        case str_hash( BIT7Z_STRING( "xz" ) ):
        case str_hash( BIT7Z_STRING( "txz" ) ):
            *format = &BitFormat::Xz;
            return true;
        case str_hash( BIT7Z_STRING( "zip" ) ):
        case str_hash( BIT7Z_STRING( "zipx" ) ):
        case str_hash( BIT7Z_STRING( "jar" ) ):
        case str_hash( BIT7Z_STRING( "xpi" ) ):
        case str_hash( BIT7Z_STRING( "odt" ) ):
        case str_hash( BIT7Z_STRING( "ods" ) ):
        case str_hash( BIT7Z_STRING( "odp" ) ):
        case str_hash( BIT7Z_STRING( "docx" ) ):
        case str_hash( BIT7Z_STRING( "xlsx" ) ):
        case str_hash( BIT7Z_STRING( "pptx" ) ):
        case str_hash( BIT7Z_STRING( "epub" ) ):
            *format = &BitFormat::Zip;
            return true;
        case str_hash( BIT7Z_STRING( "001" ) ):
            *format = &BitFormat::Split;
            return true;
        case str_hash( BIT7Z_STRING( "ar" ) ):
        case str_hash( BIT7Z_STRING( "deb" ) ):
            *format = &BitFormat::Deb;
            return true;
        case str_hash( BIT7Z_STRING( "apm" ) ):
            *format = &BitFormat::APM;
            return true;
        case str_hash( BIT7Z_STRING( "arj" ) ):
            *format = &BitFormat::Arj;
            return true;
        case str_hash( BIT7Z_STRING( "cab" ) ):
            *format = &BitFormat::Cab;
            return true;
        case str_hash( BIT7Z_STRING( "chm" ) ):
        case str_hash( BIT7Z_STRING( "chi" ) ):
            *format = &BitFormat::Chm;
            return true;
        case str_hash( BIT7Z_STRING( "msi" ) ):
        case str_hash( BIT7Z_STRING( "doc" ) ):
        case str_hash( BIT7Z_STRING( "xls" ) ):
        case str_hash( BIT7Z_STRING( "ppt" ) ):
        case str_hash( BIT7Z_STRING( "msg" ) ):
            *format = &BitFormat::Compound;
            return true;
        case str_hash( BIT7Z_STRING( "obj" ) ):
            *format = &BitFormat::COFF;
            return true;
        case str_hash( BIT7Z_STRING( "cpio" ) ):
            *format = &BitFormat::Cpio;
            return true;
        case str_hash( BIT7Z_STRING( "cramfs" ) ):
            *format = &BitFormat::CramFS;
            return true;
        case str_hash( BIT7Z_STRING( "dmg" ) ):
            *format = &BitFormat::Dmg;
            return true;
        case str_hash( BIT7Z_STRING( "dll" ) ):
        case str_hash( BIT7Z_STRING( "exe" ) ):
            //note: at least for now, we do not distinguish 7z SFX executables!
            *format = &BitFormat::Pe;
            return true;
        case str_hash( BIT7Z_STRING( "dylib" ) ):
            *format = &BitFormat::Macho;
            return true;
        case str_hash( BIT7Z_STRING( "ext" ) ):
        case str_hash( BIT7Z_STRING( "ext2" ) ):
        case str_hash( BIT7Z_STRING( "ext3" ) ):
        case str_hash( BIT7Z_STRING( "ext4" ) ):
            *format = &BitFormat::Ext;
            return true;
        case str_hash( BIT7Z_STRING( "fat" ) ):
            *format = &BitFormat::Fat;
            return true;
        case str_hash( BIT7Z_STRING( "flv" ) ):
            *format = &BitFormat::Flv;
            return true;
        case str_hash( BIT7Z_STRING( "gpt" ) ):
            *format = &BitFormat::GPT;
            return true;
        case str_hash( BIT7Z_STRING( "hfs" ) ):
        case str_hash( BIT7Z_STRING( "hfsx" ) ):
            *format = &BitFormat::Hfs;
            return true;
        case str_hash( BIT7Z_STRING( "hxs" ) ):
            *format = &BitFormat::Hxs;
            return true;
        case str_hash( BIT7Z_STRING( "ihex" ) ):
            *format = &BitFormat::IHex;
            return true;
        case str_hash( BIT7Z_STRING( "lzh" ) ):
        case str_hash( BIT7Z_STRING( "lha" ) ):
            *format = &BitFormat::Lzh;
            return true;
        case str_hash( BIT7Z_STRING( "lzma" ) ):
            *format = &BitFormat::Lzma;
            return true;
        case str_hash( BIT7Z_STRING( "lzma86" ) ):
            *format = &BitFormat::Lzma86;
            return true;
        case str_hash( BIT7Z_STRING( "mbr" ) ):
            *format = &BitFormat::Mbr;
            return true;
        case str_hash( BIT7Z_STRING( "mslz" ) ):
            *format = &BitFormat::Mslz;
            return true;
        case str_hash( BIT7Z_STRING( "mub" ) ):
            *format = &BitFormat::Mub;
            return true;
        case str_hash( BIT7Z_STRING( "nsis" ) ):
            *format = &BitFormat::Nsis;
            return true;
        case str_hash( BIT7Z_STRING( "ntfs" ) ):
            *format = &BitFormat::Ntfs;
            return true;
        case str_hash( BIT7Z_STRING( "pmd" ) ):
            *format = &BitFormat::Ppmd;
            return true;
        case str_hash( BIT7Z_STRING( "qcow" ) ):
        case str_hash( BIT7Z_STRING( "qcow2" ) ):
        case str_hash( BIT7Z_STRING( "qcow2c" ) ):
            *format = &BitFormat::QCow;
            return true;
        case str_hash( BIT7Z_STRING( "rpm" ) ):
            *format = &BitFormat::Rpm;
            return true;
        case str_hash( BIT7Z_STRING( "squashfs" ) ):
            *format = &BitFormat::SquashFS;
            return true;
        case str_hash( BIT7Z_STRING( "te" ) ):
            *format = &BitFormat::TE;
            return true;
        case str_hash( BIT7Z_STRING( "udf" ) ):
            *format = &BitFormat::Udf;
            return true;
        case str_hash( BIT7Z_STRING( "scap" ) ):
            *format = &BitFormat::UEFIc;
            return true;
        case str_hash( BIT7Z_STRING( "uefif" ) ):
            *format = &BitFormat::UEFIs;
            return true;
        case str_hash( BIT7Z_STRING( "vmdk" ) ):
            *format = &BitFormat::VMDK;
            return true;
        case str_hash( BIT7Z_STRING( "vdi" ) ):
            *format = &BitFormat::VDI;
            return true;
        case str_hash( BIT7Z_STRING( "vhd" ) ):
            *format = &BitFormat::Vhd;
            return true;
        case str_hash( BIT7Z_STRING( "xar" ) ):
        case str_hash( BIT7Z_STRING( "pkg" ) ):
            *format = &BitFormat::Xar;
            return true;
        case str_hash( BIT7Z_STRING( "z" ) ):
        case str_hash( BIT7Z_STRING( "taz" ) ):
            *format = &BitFormat::Z;
            return true;
        default:
            return false;
    }
}

/* NOTE 1: For signatures with less than 8 bytes (size of uint64_t), remaining bytes are set to 0
 * NOTE 2: Until v3, a std::unordered_map was used for mapping the signatures and the corresponding
 *         format. However, the switch case is faster and has less memory footprint. */
bool findFormatBySignature( uint64_t signature, const BitInFormat** format ) noexcept {
    constexpr auto RarSignature = 0x526172211A070000ULL; // R  a  r  !  1A 07 00
    constexpr auto Rar5Signature = 0x526172211A070100ULL; // R  a  r  !  1A 07 01 00
    constexpr auto SevenZipSignature = 0x377ABCAF271C0000ULL; // 7  z  BC AF 27 1C
    constexpr auto BZip2Signature = 0x425A680000000000ULL; // B  Z  h
    constexpr auto GZipSignature = 0x1F8B080000000000ULL; // 1F 8B 08
    constexpr auto WimSignature = 0x4D5357494D000000ULL; // M  S  W  I  M  00 00 00
    constexpr auto XzSignature = 0xFD377A585A000000ULL; // FD 7  z  X  Z  00
    constexpr auto ZipSignature = 0x504B000000000000ULL; // P  K
    constexpr auto APMSignature = 0x4552000000000000ULL; // E  R
    constexpr auto ArjSignature = 0x60EA000000000000ULL; // `  EA
    constexpr auto CabSignature = 0x4D53434600000000ULL; // M  S  C  F  00 00 00 00
    constexpr auto ChmSignature = 0x4954534603000000ULL; // I  T  S  F  03
    constexpr auto CompoundSignature = 0xD0CF11E0A1B11AE1ULL; // D0 CF 11 E0 A1 B1 1A E1
    constexpr auto CpioSignature1 = 0xC771000000000000ULL; // C7 q
    constexpr auto CpioSignature2 = 0x71C7000000000000ULL; // q  C7
    constexpr auto CpioSignature3 = 0x3037303730000000ULL; // 0  7  0  7  0
    constexpr auto DebSignature = 0x213C617263683E00ULL; // !  <  a  r  c  h  >  0A
    constexpr auto ElfSignature = 0x7F454C4600000000ULL; // 7F E  L  F
    constexpr auto PeSignature = 0x4D5A000000000000ULL; // M  Z
    constexpr auto FlvSignature = 0x464C560100000000ULL; // F  L  V  01
    constexpr auto LzmaSignature = 0x5D00000000000000ULL; //
    constexpr auto Lzma86Signature = 0x015D000000000000ULL; //
    constexpr auto MachoSignature1 = 0xCEFAEDFE00000000ULL; // CE FA ED FE
    constexpr auto MachoSignature2 = 0xCFFAEDFE00000000ULL; // CF FA ED FE
    constexpr auto MachoSignature3 = 0xFEEDFACE00000000ULL; // FE ED FA CE
    constexpr auto MachoSignature4 = 0xFEEDFACF00000000ULL; // FE ED FA CF
    constexpr auto MubSignature1 = 0xCAFEBABE00000000ULL; // CA FE BA BE 00 00 00
    constexpr auto MubSignature2 = 0xB9FAF10E00000000ULL; // B9 FA F1 0E
    constexpr auto MslzSignature = 0x535A444488F02733ULL; // S  Z  D  D  88 F0 '  3
    constexpr auto PpmdSignature = 0x8FAFAC8400000000ULL; // 8F AF AC 84
    constexpr auto QCowSignature = 0x514649FB00000000ULL; // Q  F  I  FB 00 00 00
    constexpr auto RpmSignature = 0xEDABEEDB00000000ULL; // ED AB EE DB
    constexpr auto SquashFSSignature1 = 0x7371736800000000ULL; // s  q  s  h
    constexpr auto SquashFSSignature2 = 0x6873717300000000ULL; // h  s  q  s
    constexpr auto SquashFSSignature3 = 0x7368737100000000ULL; // s  h  s  q
    constexpr auto SquashFSSignature4 = 0x7173687300000000ULL; // q  s  h  s
    constexpr auto SwfSignature = 0x4657530000000000ULL; // F  W  S
    constexpr auto SwfcSignature1 = 0x4357530000000000ULL; // C  W  S
    constexpr auto SwfcSignature2 = 0x5A57530000000000ULL; // Z  W  S
    constexpr auto TESignature = 0x565A000000000000ULL; // V  Z
    constexpr auto VMDKSignature = 0x4B444D0000000000ULL; // K  D  M  V
    constexpr auto VDISignature = 0x3C3C3C2000000000ULL; // Alternatively 0x7F10DABE at offset 0x40
    constexpr auto VhdSignature = 0x636F6E6563746978ULL; // c  o  n  e  c  t  i  x
    constexpr auto XarSignature = 0x78617221001C0000ULL; // x  a  r  !  00 1C
    constexpr auto ZSignature1 = 0x1F9D000000000000ULL; // 1F 9D
    constexpr auto ZSignature2 = 0x1FA0000000000000ULL; // 1F A0

    switch ( signature ) {
        case RarSignature:
            *format = &BitFormat::Rar;
            return true;
        case Rar5Signature:
            *format = &BitFormat::Rar5;
            return true;
        case SevenZipSignature:
            *format = &BitFormat::SevenZip;
            return true;
        case BZip2Signature:
            *format = &BitFormat::BZip2;
            return true;
        case GZipSignature:
            *format = &BitFormat::GZip;
            return true;
        case WimSignature:
            *format = &BitFormat::Wim;
            return true;
        case XzSignature:
            *format = &BitFormat::Xz;
            return true;
        case ZipSignature:
            *format = &BitFormat::Zip;
            return true;
        case APMSignature:
            *format = &BitFormat::APM;
            return true;
        case ArjSignature:
            *format = &BitFormat::Arj;
            return true;
        case CabSignature:
            *format = &BitFormat::Cab;
            return true;
        case ChmSignature:
            *format = &BitFormat::Chm;
            return true;
        case CompoundSignature:
            *format = &BitFormat::Compound;
            return true;
        case CpioSignature1:
        case CpioSignature2:
        case CpioSignature3:
            *format = &BitFormat::Cpio;
            return true;
        case DebSignature:
            *format = &BitFormat::Deb;
            return true;
            /* DMG signature detection is not this simple
            case 0x7801730D62626000:
                *format = &BitFormat::Dmg;
                return true;
            */
        case ElfSignature:
            *format = &BitFormat::Elf;
            return true;
        case PeSignature:
            *format = &BitFormat::Pe;
            return true;
        case FlvSignature:
            *format = &BitFormat::Flv;
            return true;
        case LzmaSignature:
            *format = &BitFormat::Lzma;
            return true;
        case Lzma86Signature:
            *format = &BitFormat::Lzma86;
            return true;
        case MachoSignature1:
        case MachoSignature2:
        case MachoSignature3:
        case MachoSignature4:
            *format = &BitFormat::Macho;
            return true;
        case MubSignature1:
        case MubSignature2:
            *format = &BitFormat::Mub;
            return true;
        case MslzSignature:
            *format = &BitFormat::Mslz;
            return true;
        case PpmdSignature:
            *format = &BitFormat::Ppmd;
            return true;
        case QCowSignature:
            *format = &BitFormat::QCow;
            return true;
        case RpmSignature:
            *format = &BitFormat::Rpm;
            return true;
        case SquashFSSignature1:
        case SquashFSSignature2:
        case SquashFSSignature3:
        case SquashFSSignature4:
            *format = &BitFormat::SquashFS;
            return true;
        case SwfSignature:
            *format = &BitFormat::Swf;
            return true;
        case SwfcSignature1:
        case SwfcSignature2:
            *format = &BitFormat::Swfc;
            return true;
        case TESignature:
            *format = &BitFormat::TE;
            return true;
        case VMDKSignature: // K  D  M  V
            *format = &BitFormat::VMDK;
            return true;
        case VDISignature: // Alternatively 0x7F10DABE at offset 0x40
            *format = &BitFormat::VDI;
            return true;
        case VhdSignature: // c  o  n  e  c  t  i  x
            *format = &BitFormat::Vhd;
            return true;
        case XarSignature: // x  a  r  !  00 1C
            *format = &BitFormat::Xar;
            return true;
        case ZSignature1: // 1F 9D
        case ZSignature2: // 1F A0
            *format = &BitFormat::Z;
            return true;
        default:
            return false;
    }
}

struct OffsetSignature {
    uint64_t signature;
    std::streamoff offset;
    uint32_t size;
    const BitInFormat& format;
};

const OffsetSignature common_signatures_with_offset[] = {
    { 0x504B030400000000, 0x00,  4, BitFormat::Zip },      // 50 4B 03 04
    { 0x504B030600000000, 0x00,  4, BitFormat::Zip },      // 50 4B 03 06
    { 0x377abcaf271c0000, 0x00,  6, BitFormat::SevenZip }, // 7 z BC AF 27 1C
    { 0x526172211A070100, 0x00,  7, BitFormat::Rar },      // 52 61 72 21 1A 07 00
    { 0x526172211A070000, 0x00,  8, BitFormat::Rar5 },     // 52 61 72 21 1A 07 01 00
    { 0x4D5357494D000000, 0x00,  8, BitFormat::Wim },      // 4D 53 57 49 4D 00 00 00

    { 0x2D6C680000000000, 0x02,  3, BitFormat::Lzh },    // -  l  h
    { 0x4E54465320202020, 0x03,  8, BitFormat::Ntfs },   // N  T  F  S  20 20 20 20
    { 0x4E756C6C736F6674, 0x08,  8, BitFormat::Nsis },   // N  u  l  l  s  o  f  t
    { 0x436F6D7072657373, 0x10,  8, BitFormat::CramFS }, // C  o  m  p  r  e  s  s
    { 0x7F10DABE00000000, 0x40,  4, BitFormat::VDI },    // 7F 10 DA BE
    { 0x7573746172000000, 0x101, 5, BitFormat::Tar },    // u  s  t  a  r
    // Note: since GPT files contain also the FAT signature, GPT must be checked before!
    { 0x4546492050415254, 0x200, 8, BitFormat::GPT },    // E  F  I  20 P  A  R  T
    { 0x55AA000000000000, 0x1FE, 2, BitFormat::Fat },    // U  AA
    { 0x4244000000000000, 0x400, 2, BitFormat::Hfs },    // B  D
    { 0x482B000400000000, 0x400, 4, BitFormat::Hfs },    // H  +  00 04
    { 0x4858000500000000, 0x400, 4, BitFormat::Hfs },    // H  X  00 05
    { 0x53EF000000000000, 0x438, 2, BitFormat::Ext }     // S  EF
};

uint64_t readSignature( IInStream* stream, uint32_t size ) noexcept {
    uint64_t signature = 0;
    stream->Read( &signature, size, nullptr );
    return bswap64( signature );
}

const BitInFormat& detectFormatFromSig( IInStream* stream ) {
    constexpr auto SIGNATURE_SIZE = 8U;
    constexpr auto BASE_SIGNATURE_MASK = 0xFFFFFFFFFFFFFFFFULL;
    constexpr auto BYTE_SHIFT = 8ULL;

    uint64_t file_signature = readSignature( stream, SIGNATURE_SIZE );
    uint64_t signature_mask = BASE_SIGNATURE_MASK;
    for ( auto i = 0U; i < SIGNATURE_SIZE - 1; ++i ) {
        const BitInFormat* format = nullptr;
        if ( findFormatBySignature( file_signature, &format ) ) {
            stream->Seek( 0, 0, nullptr );
            return *format;
        }
        signature_mask <<= BYTE_SHIFT;    // left shifting the mask of 1 byte, so that
        file_signature &= signature_mask; // the least significant i bytes are masked (set to 0)
    }

    for ( const auto& sig : common_signatures_with_offset ) {
        stream->Seek( sig.offset, 0, nullptr );
        file_signature = readSignature( stream, sig.size );
        if ( file_signature == sig.signature ) {
            stream->Seek( 0, 0, nullptr );
            return sig.format;
        }
    }

    // Detecting ISO/UDF
    constexpr auto ISO_SIGNATURE = 0x4344303031000000; //CD001
    constexpr auto ISO_SIGNATURE_SIZE = 5ULL;
    constexpr auto ISO_SIGNATURE_OFFSET = 0x8001;

    // Checking for ISO signature
    stream->Seek( ISO_SIGNATURE_OFFSET, 0, nullptr );
    file_signature = readSignature( stream, ISO_SIGNATURE_SIZE );
    if ( file_signature == ISO_SIGNATURE ) {
        constexpr auto MAX_VOLUME_DESCRIPTORS = 16;
        constexpr auto ISO_VOLUME_DESCRIPTOR_SIZE = 0x800; //2048

        constexpr auto UDF_SIGNATURE = 0x4E53523000000000; //NSR0
        constexpr auto UDF_SIGNATURE_SIZE = 4U;

        // The file is ISO, checking if it is also UDF!
        for ( auto descriptor_index = 1; descriptor_index < MAX_VOLUME_DESCRIPTORS; ++descriptor_index ) {
            stream->Seek( ISO_SIGNATURE_OFFSET + descriptor_index * ISO_VOLUME_DESCRIPTOR_SIZE, 0, nullptr );
            file_signature = readSignature( stream, UDF_SIGNATURE_SIZE );
            if ( file_signature == UDF_SIGNATURE ) {
                stream->Seek( 0, 0, nullptr );
                return BitFormat::Udf;
            }
        }
        stream->Seek( 0, 0, nullptr );
        return BitFormat::Iso; //No UDF volume signature found, i.e. simple ISO!
    }

    stream->Seek( 0, 0, nullptr );
    throw BitException( "Failed to detect the format of the file",
                        make_error_code( BitError::NoMatchingSignature ) );
}

#if defined(BIT7Z_USE_NATIVE_STRING) && defined(_WIN32)
#   define is_digit(ch) std::iswdigit(ch) != 0
const auto to_lower = std::towlower;
#else

inline auto is_digit( unsigned char character ) -> bool {
    return std::isdigit( character ) != 0;
}

inline auto to_lower( unsigned char character ) -> char {
    return static_cast< char >( std::tolower( character ) );
}
#endif

const BitInFormat& detectFormatFromExt( const fs::path& in_file ) {
    tstring ext = filesystem::fsutil::extension( in_file );
    if ( ext.empty() ) {
        throw BitException( "Failed to detect the archive format from the extension",
                            make_error_code( BitError::NoMatchingExtension ) );
    }
    std::transform( ext.cbegin(), ext.cend(), ext.begin(), to_lower );

    // Detecting archives with common file extensions
    const BitInFormat* format = nullptr;
    if ( findFormatByExtension( ext, &format ) ) { //extension found in map
        return *format;
    }

    // Detecting multi-volume archives extensions
    if ( ( ext[ 0 ] == BIT7Z_STRING( 'r' ) || ext[ 0 ] == BIT7Z_STRING( 'z' ) ) &&
         ( ext.size() == 3 && is_digit( ext[ 1 ] ) && is_digit( ext[ 2 ] ) ) ) {
        // Extension follows the format zXX or rXX, where X is a number in range [0-9]
        return ext[ 0 ] == BIT7Z_STRING( 'r' ) ? BitFormat::Rar : BitFormat::Zip;
    }

    // Note: iso, img, and ima file extensions can be associated with different formats -> detect by signature.

    // The extension did not match any known format extension, delegating the decision to the client.
    return BitFormat::Auto;
}
}  // namespace bit7z

#endif
