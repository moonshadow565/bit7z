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

#ifndef _WIN32

#include <catch2/catch.hpp>

#include <array>
#include <cstring>
#include <iostream>

#include <internal/windows.hpp>

TEST_CASE( "winapi: Checking error code macros correct values", "[winapi]" ) {
#ifdef MY__E_ERROR_NEGATIVE_SEEK
    REQUIRE( HRESULT_WIN32_ERROR_NEGATIVE_SEEK == MY__E_ERROR_NEGATIVE_SEEK );
#else
    REQUIRE( HRESULT_WIN32_ERROR_NEGATIVE_SEEK == HRESULT_FROM_WIN32(ERROR_NEGATIVE_SEEK) );
#endif
}

TEST_CASE( "winapi: Allocating BSTR string from nullptr C strings", "[winapi][string allocation]" ) {
    SECTION( "Using SysAllocString" ) {
        REQUIRE( SysAllocString( nullptr ) == nullptr );
    }

    SECTION( "Using a specific length" ) {
        BSTR result_string;
        size_t test_length = GENERATE( 0, 1, 42, 127, 128 );

        DYNAMIC_SECTION( "SysAllocStringLen with length " << test_length ) {
            result_string = SysAllocStringLen( nullptr, test_length );
            REQUIRE( SysStringLen( result_string ) == test_length );
            REQUIRE( SysStringByteLen( result_string ) == test_length * sizeof( std::remove_pointer< BSTR >::type ) );
        }

        DYNAMIC_SECTION( "SysAllocStringByteLen with byte length " << test_length ) {
            result_string = SysAllocStringByteLen( nullptr, test_length );
            REQUIRE( SysStringLen( result_string ) == test_length / sizeof( std::remove_pointer< BSTR >::type ) );
            REQUIRE( SysStringByteLen( result_string ) == test_length );
        }

        REQUIRE( result_string != nullptr );
        REQUIRE( result_string == std::wstring{} );
        REQUIRE_NOTHROW( SysFreeString( result_string ) );
    }

    SECTION( "Using the max value for the length type" ) {
        auto length = std::numeric_limits< UINT >::max();
        REQUIRE( SysAllocStringLen( nullptr, length ) == nullptr );
        REQUIRE( SysAllocStringByteLen( nullptr, length ) == nullptr );
    }

    SECTION( "Using a length value that wraps around" ) {
        auto length = 0xC0000000;
        REQUIRE( SysAllocStringLen( nullptr, length ) == nullptr );

        length = ( std::numeric_limits< UINT >::max() / sizeof( OLECHAR ) );
        REQUIRE( SysAllocStringLen( nullptr, length ) == nullptr );

        length = 0x30000000;
        BSTR string = SysAllocStringLen( nullptr, length );
        REQUIRE( string != nullptr );
        REQUIRE( string == std::wstring{} );
        REQUIRE( SysStringLen( string ) == length );
        REQUIRE_NOTHROW( SysFreeString( string ) );
    }
}

TEST_CASE( "winapi: Handling nullptr BSTR strings", "[winapi][nullptr BSTR]" ) {
    REQUIRE( SysStringByteLen( nullptr ) == 0 );
    REQUIRE( SysStringLen( nullptr ) == 0 );
    REQUIRE_NOTHROW( SysFreeString( nullptr ) );
}

TEST_CASE( "winapi: Allocating from wide strings", "[winapi][string allocation]" ) {
    auto test_str = GENERATE( as< const wchar_t* >(),
                              L"",
                              L"h",
                              L"hello world!",
                              L"supercalifragilistichespiralidoso",
                              L"perché",
                              L"\u30e1\u30bf\u30eb\u30ac\u30eb\u30eb\u30e2\u30f3" // メタルガルルモン
    );

    DYNAMIC_SECTION( "Testing L" << Catch::StringMaker< std::wstring >::convert( test_str ) << " wide string" ) {
        std::wstring expected_string;
        BSTR result_string = nullptr;

        SECTION( "SysAllocString" ) {
            expected_string = test_str;
            result_string = SysAllocString( test_str );
        }

        SECTION( "SysAllocStringLen" ) {
            expected_string = test_str;
            result_string = SysAllocStringLen( test_str, expected_string.size() );
        }

        SECTION( "SysAllocStringLen with half-length parameter" ) {
            expected_string = std::wstring{ test_str, std::wcslen( test_str ) / 2 };
            result_string = SysAllocStringLen( test_str, expected_string.size() );
        }

        SECTION( "SysAllocStringLen with zero length parameter" ) {
            // expected_string is already empty here
            result_string = SysAllocStringLen( test_str, 0 );
        }

        REQUIRE( result_string != nullptr );
        REQUIRE( result_string == expected_string );
        REQUIRE( SysStringLen( result_string ) == expected_string.size() );
        REQUIRE( SysStringByteLen( result_string ) == ( expected_string.size() * sizeof( OLECHAR ) ) );

        REQUIRE_NOTHROW( SysFreeString( result_string ) );
    }
}

TEST_CASE( "winapi: Allocating from narrow strings", "[winapi][string allocation]" ) {
    auto test_str = GENERATE( as< const char* >(),
                              "",
                              "h",
                              "hello world!",
                              "supercalifragilistichespiralidoso",
                              "perché",
                              "\u30e1\u30bf\u30eb\u30ac\u30eb\u30eb\u30e2\u30f3" // メタルガルルモン
    );

    DYNAMIC_SECTION( "Testing " << Catch::StringMaker< std::string >::convert( test_str ) << " string" ) {
        BSTR result_string = nullptr;
        std::string expected_string;

        SECTION( "SysAllocStringByteLen" ) {
            expected_string = test_str;
            result_string = SysAllocStringByteLen( test_str, expected_string.size() );
        }

        SECTION( "SysAllocStringByteLen with half-length parameter" ) {
            expected_string = std::string{ test_str, std::strlen( test_str ) / 2 };
            result_string = SysAllocStringByteLen( test_str, expected_string.size() );
        }

        SECTION( "SysAllocStringByteLen with zero length parameter" ) {
            // expected_string is already empty here
            result_string = SysAllocStringByteLen( test_str, 0 );
        }

        REQUIRE( result_string != nullptr );
        REQUIRE( reinterpret_cast< const char* >( result_string ) == expected_string );
        REQUIRE( SysStringLen( result_string ) == ( expected_string.size() / sizeof( OLECHAR ) ) );
        REQUIRE( SysStringByteLen( result_string ) == expected_string.size() );

        REQUIRE_NOTHROW( SysFreeString( result_string ) );
    }
}

#endif