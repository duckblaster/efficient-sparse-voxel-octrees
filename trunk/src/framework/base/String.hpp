/*
 *  Copyright 2009-2010 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
 
#pragma once
#include "base/Array.hpp"

#include <stdarg.h>

namespace FW
{
//------------------------------------------------------------------------

class String
{
public:
                    String      (void)                          {}
                    String      (char chr)                      { set(chr); }
                    String      (const char* chars)             { set(chars); }
                    String      (const String& other)           { set(other); }
                    String      (S32 value)                     { setf("%d", value); }
                    String      (F64 value)                     { setf("%g", value); }
                    ~String     (void)                          {}

    int             getLength   (void) const                    { return max(m_chars.getSize() - 1, 0); }
    char            getChar     (int idx) const                 { FW_ASSERT(idx < getLength()); return m_chars[idx]; }
    const char*     getPtr      (void) const                    { return (m_chars.getSize()) ? m_chars.getPtr() : ""; }

    String&         reset       (void)                          { m_chars.reset(); return *this; }
    String&         set         (char chr);
    String&         set         (const char* chars);
    String&         set         (const String& other)           { m_chars = other.m_chars; return *this; }
    String&         setf        (const char* fmt, ...);
    String&         setfv       (const char* fmt, va_list args);

    String          substring   (int start, int end) const;
    String          substring   (int start) const               { return substring(start, getLength()); }

    String&         clear       (void)                          { m_chars.clear(); }
    String&         append      (char chr);
    String&         append      (const char* chars);
    String&         append      (const String& other);
    String&         appendf     (const char* fmt, ...);
    String&         appendfv    (const char* fmt, va_list args);
    String&         compact     (void)                          { m_chars.compact(); }

    int             indexOf     (char chr) const                { return m_chars.indexOf(chr); }
    int             indexOf     (char chr, int fromIdx) const   { return m_chars.indexOf(chr, fromIdx); }
    int             lastIndexOf (char chr) const                { return m_chars.lastIndexOf(chr); }
    int             lastIndexOf (char chr, int fromIdx) const   { return m_chars.lastIndexOf(chr, fromIdx); }

    String          toUpper     (void) const;
    String          toLower     (void) const;
    bool            startsWith  (const String& str) const;
    bool            endsWith    (const String& str) const;

    String          getFileName (void) const;
    String          getDirName  (void) const;

    char            operator[]  (int idx) const                 { return getChar(idx); }
    String&         operator=   (const String& other)           { set(other); return *this; }
    String&         operator+=  (char chr)                      { append(chr); return *this; }
    String&         operator+=  (const String& other)           { append(other); return *this; }
    String          operator+   (char chr) const                { return String(*this).append(chr); }
    String          operator+   (const String& other) const     { return String(*this).append(other); }
    bool            operator==  (const char* chars) const       { return (strcmp(getPtr(), chars) == 0); }
    bool            operator==  (const String& other) const     { return (strcmp(getPtr(), other.getPtr()) == 0); }
    bool            operator!=  (const char* chars) const       { return (strcmp(getPtr(), chars) != 0); }
    bool            operator!=  (const String& other) const     { return (strcmp(getPtr(), other.getPtr()) != 0); }
    bool            operator<   (const char* chars) const       { return (strcmp(getPtr(), chars) < 0); }
    bool            operator<   (const String& other) const     { return (strcmp(getPtr(), other.getPtr()) < 0); }
    bool            operator>   (const char* chars) const       { return (strcmp(getPtr(), chars) > 0); }
    bool            operator>   (const String& other) const     { return (strcmp(getPtr(), other.getPtr()) > 0); }
    bool            operator>=  (const char* chars) const       { return (strcmp(getPtr(), chars) <= 0); }
    bool            operator>=  (const String& other) const     { return (strcmp(getPtr(), other.getPtr()) <= 0); }
    bool            operator<=  (const char* chars) const       { return (strcmp(getPtr(), chars) >= 0); }
    bool            operator<=  (const String& other) const     { return (strcmp(getPtr(), other.getPtr()) >= 0); }

private:
    static int      strlen      (const char* chars);
    static int      strcmp      (const char* a, const char* b);

private:
    Array<char>     m_chars;
};

//------------------------------------------------------------------------

bool    parseSpace      (const char*& ptr);
bool    parseChar       (const char*& ptr, char chr);
bool    parseLiteral    (const char*& ptr, const char* str);
bool    parseInt        (const char*& ptr, S32& value);
bool    parseFloat      (const char*& ptr, F32& value);

//------------------------------------------------------------------------
}
