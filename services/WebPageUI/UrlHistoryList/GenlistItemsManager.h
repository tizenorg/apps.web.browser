/*
 * GenlistItemsManager.h
 *
 *  Created on: Oct 23, 2015
 *      Author: a.skobodzins
 */

#ifndef GENLISTITEMSMANAGER_H_
#define GENLISTITEMSMANAGER_H_

#include <memory>
#include <map>

#include <Elementary.h>

using namespace std;

namespace tizen_browser {
namespace base_ui {

enum class GenlistItemType
{
    ITEM_CURRENT, ITEM_FIRST, ITEM_LAST, ITEM_SPACE_FIRST, ITEM_SPACE_LAST
};

/**
 * Stores and manipulated pointers on Elm_Object_Item for GenlistManager
 */
class GenlistItemsManager
{
public:
    GenlistItemsManager();
    virtual ~GenlistItemsManager();

    Elm_Object_Item* getItem(GenlistItemType type);
    void setItems(std::initializer_list<GenlistItemType> types,
            Elm_Object_Item* item);
    /**
     * Same as #setItems, except only nullptr value pointers are set
     */
    void setItemsIfNullptr(std::initializer_list<GenlistItemType> types,
            Elm_Object_Item* item);
    /**
     * Assign src pointer value to dst.
     */
    void assignItem(GenlistItemType dst, GenlistItemType src);
    /**
     * Assign item of a given type to a elm_genlist_item_next_get item, if
     * there is one. Return false, if value has not changed.
     */
    bool shiftItemDown(GenlistItemType item);
    bool shiftItemUp(GenlistItemType item);

private:
    map<GenlistItemType, shared_ptr<Elm_Object_Item*>> ptrMap;

};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* GENLISTITEMSMANAGER_H_ */
