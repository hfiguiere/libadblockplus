/*
 * This file is part of Adblock Plus <https://adblockplus.org/>,
 * Copyright (C) 2006-2015 Eyeo GmbH
 *
 * Adblock Plus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * Adblock Plus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Adblock Plus.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ADBLOCK_PLUS_OBJC_HELPER_H
#define ADBLOCK_PLUS_OBJC_HELPER_H

namespace AdblockPlus
{
  /*
   * Downcasts the passed parameter to the template type if it is an instance of template type T.
   * @param Pointer to the base class.
   * @return Valid pointer or nil if the parameter is not an instance of the passed class.
   */
  template<typename T> inline T* ObjcCast(id from)
  {
    if ([from isKindOfClass:[T class]])
    {
      return static_cast<T*>(from);
    }
    return nil;
  }
  
  /*
   * Converts NSData to NSString by copying the data.
   * @param NSData* source to convert. Can be nil or zero length buffer.
   * @return Created NSString* if data is not nil neither zero length buffer, otherwise returns nil.
   */
  inline NSString* ToNSString(NSData* data)
  {
    NSString* stringData = nil;
    if (data)
      if (const void* bytesData = [data bytes])
        stringData =@(static_cast<const char*>(bytesData));
    return stringData;
  }
  
  /*
   * Converts C++ `bool` value to Objective-C `BOOL`.
   */
  inline BOOL ToObjCBOOL(bool value)
  {
    return value ? YES : NO;
  }
  
  /*
   * Converts int64_t value to int32_t. JavaScriptCore does not support 64-bit integers and this
   * method allows to track in future when we needed it.
   */
  inline int32_t ToInt32(int64_t value)
  {
    return static_cast<int32_t>(value);
  }
}

#endif // ADBLOCK_PLUS_OBJC_HELPER_H
