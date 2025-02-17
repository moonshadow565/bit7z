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

#include "bitarchiveiteminfo.hpp"

using bit7z::BitArchiveItemInfo;
using bit7z::BitProperty;
using bit7z::BitPropVariant;
using std::map;

BitArchiveItemInfo::BitArchiveItemInfo( uint32_t item_index ) : BitArchiveItem( item_index ) {}

BitPropVariant BitArchiveItemInfo::itemProperty( BitProperty property ) const {
    auto prop_it = mItemProperties.find( property );
    return ( prop_it != mItemProperties.end() ? ( *prop_it ).second : BitPropVariant() );
}

map< BitProperty, BitPropVariant > BitArchiveItemInfo::itemProperties() const {
    return mItemProperties;
}

void BitArchiveItemInfo::setProperty( BitProperty property, const BitPropVariant& value ) {
    mItemProperties[ property ] = value;
}
