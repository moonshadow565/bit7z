// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
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

#include "../include/updatecallback.hpp"

#include "../include/util.hpp"

using namespace bit7z;

UpdateCallback::UpdateCallback( const BitArchiveCreator& creator )
    : Callback{ creator },
      mOldArc{ nullptr },
      mOldArcItemsCount{ 0 },
      mRenamedItems{ nullptr },
      mAskPassword{ false },
      mNeedBeClosed{ false } {}

UpdateCallback::~UpdateCallback() {
    Finalize();
}

void UpdateCallback::setOldArc( const BitInputArchive* old_arc ) {
    if ( old_arc != nullptr ) {
        mOldArc = old_arc;
        mOldArcItemsCount = old_arc->itemsCount();
    }
}

void UpdateCallback::setRenamedItems( RenamedItems renamed_items ) {
    mRenamedItems = std::make_unique< const RenamedItems >( std::move( renamed_items ) );
}

HRESULT UpdateCallback::Finalize() {
    if ( mNeedBeClosed ) {
        mNeedBeClosed = false;
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetTotal( UInt64 size ) {
    if ( mHandler.totalCallback() ) {
        mHandler.totalCallback()( size );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetCompleted( const UInt64* completeValue ) {
    if ( completeValue != nullptr && mHandler.progressCallback() ) {
        return mHandler.progressCallback()( *completeValue ) ? S_OK : E_ABORT;
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetRatioInfo( const UInt64* inSize, const UInt64* outSize ) {
    if ( inSize != nullptr && outSize != nullptr && mHandler.ratioCallback() ) {
        mHandler.ratioCallback()( *inSize, *outSize );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetProperty( UInt32 index, PROPID propID, PROPVARIANT* value ) {
    BitPropVariant prop;
    if ( propID == kpidIsAnti ) {
        prop = false;
    } else if ( index < mOldArcItemsCount ) {
        if ( mRenamedItems != nullptr && propID == kpidPath ) {
            auto res = mRenamedItems->find( index );
            if ( res != mRenamedItems->end() ) {
                prop = WIDEN( res->second );
            }
        }
        if ( prop.isEmpty() ) { //Not renamed
            prop = mOldArc->getItemProperty( index, static_cast< BitProperty >( propID ) );
        }
    } else {
        prop = getNewItemProperty( index, propID );
    }
    *value = prop;
    prop.bstrVal = nullptr;
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetStream( UInt32 index, ISequentialInStream** inStream ) {
    RINOK( Finalize() )

    if ( index < mOldArcItemsCount ) { //old item in the archive
        return S_OK;
    }

    return getNewItemStream( index, inStream );
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetVolumeSize( UInt32 /*index*/, UInt64* /*size*/ ) {
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetVolumeStream( UInt32 /*index*/, ISequentialOutStream** /*volumeStream*/ ) {
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetUpdateItemInfo( UInt32 index,
                                                Int32* newData,
                                                Int32* newProperties,
                                                UInt32* indexInArchive ) {

    bool isOldItem = index < mOldArcItemsCount;
    bool isRenamedItem = mRenamedItems != nullptr && mRenamedItems->find( index ) != mRenamedItems->end();

    if ( newData != nullptr ) {
        *newData = isOldItem ? 0 : 1; //= true;
    }
    if ( newProperties != nullptr ) {
        *newProperties = isOldItem && !isRenamedItem ? 0 : 1; //= true;
    }
    if ( indexInArchive != nullptr ) {
        *indexInArchive = isOldItem ? index : static_cast< uint32_t >( -1 );
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetOperationResult( Int32 /* operationResult */ ) {
    mNeedBeClosed = true;
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::CryptoGetTextPassword2( Int32* passwordIsDefined, BSTR* password ) {
    if ( !mHandler.isPasswordDefined() ) {
        if ( mAskPassword ) {
            // You can ask real password here from user
            // Password = GetPassword(OutStream);
            // PasswordIsDefined = true;
            mErrorMessage = kPasswordNotDefined;
            return E_ABORT;
        }
    }

    *passwordIsDefined = ( mHandler.isPasswordDefined() ? 1 : 0 );
    return StringToBstr( WIDEN( mHandler.password() ).c_str(), password );
}
