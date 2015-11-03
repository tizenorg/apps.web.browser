/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HISTORYMATCHFINDER_H_
#define HISTORYMATCHFINDER_H_

#include <string>
#include <vector>

#include "HistoryItem.h"

using namespace std;

namespace tizen_browser {
namespace services {

/**
 * @brief removes history items not matching given keywords
 * @param historyItems vector from which mismatching items will be removed
 * @param keywords keywords (history item is a match, when all keywords are
 * matching)
 */
void removeMismatches(const std::shared_ptr<HistoryItemVector>& historyItems,
        const vector<string>& keywords);

} /* namespace services */
} /* namespace tizen_browser */

#endif /* HISTORYMATCHFINDER_H_ */
