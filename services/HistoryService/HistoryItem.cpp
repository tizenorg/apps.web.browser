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

#include "HistoryItem.h"

namespace tizen_browser
{
namespace services
{

HistoryItem::HistoryItem(HistoryItem && other) throw()
{
    *this = std::move(other);
}

HistoryItem::HistoryItem(const std::string & url,
                         const std::string & title,
                         std::shared_ptr<tizen_browser::tools::BrowserImage> image)
    : m_url(url)
    , m_title(title)
    , m_lastVisit()
    , m_favIcon(image)
{
}

HistoryItem::HistoryItem(const std::string & url)
    : m_url(url)
    , m_title()
    , m_lastVisit()
    , m_favIcon(std::make_shared<tizen_browser::tools::BrowserImage>())
{

}

HistoryItem::HistoryItem(const HistoryItem& source)
    : m_url(source.m_url)
    , m_title(source.m_title)
    , m_lastVisit(source.m_lastVisit)
    , m_favIcon(source.m_favIcon)
    , m_visitCounter(source.m_visitCounter)
{

}

HistoryItem::~HistoryItem()
{

}

HistoryItem & HistoryItem::operator=(HistoryItem && other) throw()
{
    if (this != &other) {
        m_url = std::move(other.m_url);
        m_title = std::move(other.m_title);
        m_visitDate = std::move(other.m_visitDate);
        m_visitCounter = std::move(other.m_visitCounter);
        m_favIcon = std::move(other.m_favIcon);
    }
    return *this;
}

bool HistoryItem::operator==(const HistoryItem& other)
{
    return (m_url == other.m_url);
}

bool HistoryItem::operator!=(const HistoryItem& other)
{
    return (m_url != other.m_url);
}

void HistoryItem::setUrl(const std::string & url)
{
    m_url = url;
}

std::string HistoryItem::getUrl() const
{
    return m_url;
}

void HistoryItem::setTitle(const std::string & title)
{
    m_title = title;
}

std::string HistoryItem::getTitle() const
{
    return m_title;
}

// void HistoryItem::setVisitDate(boost::gregorian::date visitDate)
// {
//     m_visitDate = visitDate;
// }
//
// boost::gregorian::date HistoryItem::getVisitDate()
// {
//     return m_visitDate;
// }

void HistoryItem::setLastVisit(boost::posix_time::ptime visitDate)
{
    m_lastVisit = visitDate;
}

boost::posix_time::ptime HistoryItem::getLastVisit() const
{
    return m_lastVisit;
}

void HistoryItem::setVisitCounter(int visitCounter)
{
    m_visitCounter = visitCounter;
}

int HistoryItem::getVisitCounter()
{
    return m_visitCounter;
}

void HistoryItem::setFavIcon(std::shared_ptr<tizen_browser::tools::BrowserImage> favIcon)
{
    m_favIcon = favIcon;
}

std::shared_ptr<tizen_browser::tools::BrowserImage> HistoryItem::getFavIcon()
{
    return m_favIcon;
}

void HistoryItem::setThumbnail(std::shared_ptr<tizen_browser::tools::BrowserImage> thumbnail)
{
    m_thumbnail = thumbnail;
};

std::shared_ptr<tizen_browser::tools::BrowserImage> HistoryItem::getThumbnail() const
{

    return m_thumbnail;
};

void HistoryItem::setUriFavicon(const std::string & uri) {
    m_urifavicon = uri;
}

std::string HistoryItem::getUriFavicon() {
    return m_urifavicon;
}


}
}
