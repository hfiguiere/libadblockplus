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

#ifndef ADBLOCK_PLUS_DECLARATIONS_H
#define ADBLOCK_PLUS_DECLARATIONS_H

#include <memory>
#include <vector>
#include <functional>

namespace AdblockPlus
{
  class JsValue;
  class JsEngine;

  /**
   * Shared pointer to `JsEngine` instance.
   */
  typedef std::shared_ptr<JsEngine> JsEnginePtr;
  
  /**
   * Shared smart pointer to a `JsValue` instance.
   */
  typedef std::shared_ptr<JsValue> JsValuePtr;
  
  /**
   * List of JavaScript values.
   */
  typedef std::vector<AdblockPlus::JsValuePtr> JsValueList;
}
#endif // ADBLOCK_PLUS_DECLARATIONS_H
