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

#ifndef BITARCHIVEITEM_HPP
#define BITARCHIVEITEM_HPP

#include "bitgenericitem.hpp"

namespace bit7z {

    class BitArchiveItem : public BitGenericItem {
        public:
            /**
             * @brief BitArchiveItem destructor.
             */
            ~BitArchiveItem() override = default;

            /**
             * @return the index of the item in the archive.
             */
            BIT7Z_NODISCARD uint32_t index() const noexcept;

            /**
             * @return true if and only if the item is a directory (i.e. it has the property BitProperty::IsDir).
             */
            BIT7Z_NODISCARD bool isDir() const override;

            /**
             * @return the name of the item, if available or inferable from the path, or an empty string otherwise.
             */
            BIT7Z_NODISCARD tstring name() const override;

            /**
             * @return the extension of the item, if available or inferable from the name, or an empty string otherwise
             * (e.g. when the item is a folder).
             */
            BIT7Z_NODISCARD tstring extension() const;

            /**
             * @return the path of the item in the archive, if available or inferable from the name, or an empty string
             * otherwise.
             */
            BIT7Z_NODISCARD tstring path() const override;

            /**
             * @return the uncompressed size of the item.
             */
            BIT7Z_NODISCARD uint64_t size() const override;

            BIT7Z_NODISCARD FILETIME creationTime() const override;

            BIT7Z_NODISCARD FILETIME lastAccessTime() const override;

            BIT7Z_NODISCARD FILETIME lastWriteTime() const override;

            BIT7Z_NODISCARD uint32_t attributes() const override;

            /**
             * @return the compressed size of the item.
             */
            BIT7Z_NODISCARD uint64_t packSize() const;

            /**
             * @return true if and only if the item is encrypted.
             */
            BIT7Z_NODISCARD bool isEncrypted() const;

        protected:
            uint32_t mItemIndex; //Note: it is not const since sub-class BitArchiveItemOffset can increment it!

            explicit BitArchiveItem( uint32_t item_index ) noexcept;
    };

}

#endif // BITARCHIVEITEM_HPP
