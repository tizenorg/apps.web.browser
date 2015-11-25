#ifndef __ABSTRACT_POPUP_H__
#define __ABSTRACT_POPUP_H__ 1

namespace tizen_browser {
namespace interfaces {

/**
 * @brief This is common popup interface
 */
class AbstractPopup
{
public:
    /**
     * @brief static method to create popup instance
     * @return popup instance
     */
    static AbstractPopup* createPopup();

    /**
     * @brief virtual method to show popup
     * Note that it may require to notify some popup manager
     */
    virtual void show() = 0;

    /**
     * @brief virtual method to close popup
     * Note that it may require to notify some popup manager
     */
    virtual void dismiss() = 0;

    /**
     * @brief virtual method to handle back key pressed
     * Note that in most cases it will just call dismiss()
     */
    virtual void onBackPressed() = 0;

    virtual ~AbstractPopup() {};
};

} //iterfaces
} //tizen_browser 

#endif // __ABSTRACT_POPUP_H__
