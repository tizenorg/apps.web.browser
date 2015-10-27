/*
 * GenlistItemsManager.cpp
 *
 *  Created on: Oct 23, 2015
 *      Author: a.skobodzins
 */

#include "GenlistItemsManager.h"
#include "BrowserLogger.h"

namespace tizen_browser {
namespace base_ui {

GenlistItemsManager::GenlistItemsManager()
{
    ptrMap.insert( { GenlistItemType::ITEM_CURRENT,
        make_shared<Elm_Object_Item*>() });
    ptrMap.insert( { GenlistItemType::ITEM_FIRST,
        make_shared<Elm_Object_Item*>() });
    ptrMap.insert({ GenlistItemType::ITEM_LAST,
        make_shared<Elm_Object_Item*>() });
    ptrMap.insert({ GenlistItemType::ITEM_SPACE_FIRST,
        make_shared<Elm_Object_Item*>() });
    ptrMap.insert({ GenlistItemType::ITEM_SPACE_LAST,
        make_shared<Elm_Object_Item*>() });
}

GenlistItemsManager::~GenlistItemsManager()
{
}

Elm_Object_Item* GenlistItemsManager::getItem(GenlistItemType type)
{
    return *ptrMap.at(type);
}

void GenlistItemsManager::setItems(std::initializer_list<GenlistItemType> types,
        Elm_Object_Item* item)
{
    for (auto i : types) {
        *ptrMap.at(i) = item;
    }
}

void GenlistItemsManager::setItemsIfNullptr(
        std::initializer_list<GenlistItemType> types, Elm_Object_Item* item)
{
    for (auto i : types) {
        if (!getItem(i)) {
            setItems( { i }, item);
        }
    }
}

void GenlistItemsManager::assignItem(GenlistItemType dst, GenlistItemType src)
{
    setItems( { dst }, getItem(src));
}

bool GenlistItemsManager::shiftItemDown(GenlistItemType item)
{
    if (!getItem(item))
        return false;
    Elm_Object_Item* item_next = elm_genlist_item_next_get(getItem(item));
    if (item_next) {
        setItems( { item }, item_next);
        return true;
    }
    return false;
}

bool GenlistItemsManager::shiftItemUp(GenlistItemType item)
{
    if (!getItem(item))
        return false;
    Elm_Object_Item* item_prev = elm_genlist_item_prev_get(getItem(item));
    if (item_prev) {
        setItems( { item }, item_prev);
        return true;
    }
    return false;
}

} /* namespace base_ui */
} /* namespace tizen_browser */
