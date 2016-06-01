/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#ifndef __TAB_ID_H__
#define __TAB_ID_H__ 1

#include <string>
#include <memory>

#include "BrowserImageTypedef.h"
#include "TabIdTypedef.h"

namespace tizen_browser {
namespace basic_webengine {

/**
 * Class TabId is designed for using as Tab ID.
 * Now it is a simple wrapper for int, but it can be replaced with more complex solution
 */
class TabId
{
public:
    TabId(int id);
    TabId(const TabId & n);
    virtual ~TabId();

    virtual bool operator==(const TabId & n) const {
        return m_id == n.m_id;
    }
    virtual bool operator!=(const TabId & n) const {
        return m_id != n.m_id;
    }
    virtual bool operator>(const TabId & n) const {
        return m_id > n.m_id;
    }
    virtual bool operator<(const TabId & n) const {
        return m_id < n.m_id;
    }

    static TabId NONE;

    int get() const {
        return m_id;
    };

    /// to string conversation - required only for DEBUG
    virtual std::string toString() const;
private:
    int m_id;
};

class TabContent
{
public:
    TabContent(TabId id, const std::string& title,
            tools::BrowserImagePtr thumbnail);
    TabContent(TabId id, const std::string& title);
    TabId getId() const;
    std::string getTitle() const;
    void setThumbnail(tools::BrowserImagePtr thumbnail);
    tools::BrowserImagePtr getThumbnail() const;
    void setIsCurrentTab( bool isCurrentTab ) { m_isCurrentTab = isCurrentTab; };
    bool getIsCurrentTab() { return m_isCurrentTab; };

private:
    TabId m_id;
    std::string m_title;
    tools::BrowserImagePtr m_thumbnail;
    bool m_isCurrentTab;

};
} /* end of basic_webengine */
} /* end of tizen_browser */

#endif /* __TAB_ID_H__ */
