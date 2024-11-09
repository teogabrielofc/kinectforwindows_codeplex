//------------------------------------------------------------------------------
// <copyright file="Utility.h" company="Microsoft">
// 	 
//	 Copyright 2013 Microsoft Corporation 
// 	 
//	Licensed under the Apache License, Version 2.0 (the "License"); 
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
// 	 
//		 http://www.apache.org/licenses/LICENSE-2.0 
// 	 
//	Unless required by applicable law or agreed to in writing, software 
//	distributed under the License is distributed on an "AS IS" BASIS,
//	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
//	See the License for the specific language governing permissions and 
//	limitations under the License. 
// 	 
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include <Windows.h>

#define CLOSING_FROM_STATUSCHANGED    1
#define READING_TEXT_BUFFER_SIZE    128

/// <summary>
/// Ensure the font object has been created correctly
/// </summary>
inline void EnsureFontCreated(HFONT& hFont, int fontSize, int fontWeight)
{
    if (nullptr == hFont)
    {
        hFont = CreateFontW(fontSize, 0, 0, 0, fontWeight, FALSE, FALSE, FALSE, 
            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    }
}

/// <summary>
/// Ensure the image object has been loaded correctly
/// </summary>
inline void EnsureImageLoaded(HBITMAP& hBitmap, UINT resourceId)
{
    if (nullptr == hBitmap)
    {
        hBitmap = LoadBitmapW(GetModuleHandle(NULL), MAKEINTRESOURCEW(resourceId));
    }
}

/// <summary>
/// Ensure the icon object has been loaded correctly
/// </summary>
inline void EnsureIconLoaded(HICON& hIcon, UINT resourceId)
{
    if (nullptr == hIcon)
    {
        hIcon = LoadIconW(GetModuleHandle(NULL), MAKEINTRESOURCEW(resourceId));
    }
}

inline SIZE GetWindowSize(HWND hWnd)
{
    RECT rect;
    GetWindowRect(hWnd, &rect);

    SIZE size = {rect.right - rect.left, rect.bottom - rect.top};
    return size;
}

inline SIZE GetClientSize(HWND hWnd)
{
    RECT rect;
    GetClientRect(hWnd, &rect);

    SIZE size = {rect.right, rect.bottom};
    return size;
}

// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface*& pInterfaceToRelease)
{
    if (pInterfaceToRelease)
    {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = nullptr;
    }
}

// Safe delete pointers
template<class T>
inline void SafeDelete(T*& ptr)
{
    if(ptr)
    {
        delete ptr;
        ptr = nullptr;
    }
}

// Safe delete array
template<class T>
inline void SafeDeleteArray(T*& pArray)
{
    if(pArray)
    {
        delete[] pArray;
        pArray = nullptr;
    }
}

// Compare the reading, format and update to static control if it changed
template<class T>
void CompareUpdateValue(const T newValue, T& storedValue, HWND hWnd, UINT controlID, LPCWSTR format)
{
    if(storedValue != newValue)
    {
        storedValue = newValue;

        WCHAR buffer[READING_TEXT_BUFFER_SIZE];
        swprintf_s(buffer, READING_TEXT_BUFFER_SIZE, format, newValue);
        SetDlgItemTextW(hWnd, controlID, buffer);
    }
}
