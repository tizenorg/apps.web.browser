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

#include <boost/uuid/uuid_io.hpp>

#include "BrowserImage.h"

namespace tizen_browser {
namespace basic_webengine {

/**
 * Class TabId is designed for using as Tab ID.
 * Now it is a simple wrapper for int, but it can be replaced with more complex solution
 */
class TabId
{
public:
    /**
     * @brief Default constructor - generates random initial UUID
     */
    TabId();
    /// Copy constructor
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

    /// to string conversation - required only for DEBUG
    virtual std::string toString() const;
private:
    boost::uuids::uuid m_id;
};

class TabContent{
public:
    TabContent(TabId id,const std::string& title, std::shared_ptr<tizen_browser::tools::BrowserImage> thumbnail);
    TabId getId() const;
    std::string getTitle() const;
    std::shared_ptr<tizen_browser::tools::BrowserImage> getThumbnail() const;
private:
    TabId m_id;
    std::string m_title;
    std::shared_ptr<tizen_browser::tools::BrowserImage> m_thumbnail;
};
} /* end of basic_webengine */
} /* end of tizen_browser */

#endif /* __TAB_ID_H__ */
