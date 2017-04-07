#include <Arduino.h>

#ifndef menuItem_h
#define menuItem_h

typedef void (* MENU_ACTION_CALLBACK_FUNC)( char * pMenuText, void * pUserData );

void menuItem_BoolTrueCallbackFunc( char * pMenuText, void * pUserData );
void menuItem_BoolFalseCallbackFunc( char * pMenuText, void * pUserData );
void menuItem_BackCallbackFunc( char * pMenuText, void * pUserData );

class menuItem {

public:

menuItem(char * menuText, void * userData, MENU_ACTION_CALLBACK_FUNC func);
bool addChild(menuItem* child);
void addSibling(menuItem* sibling);
void setPrevSibling(menuItem* prevSibling);
void addActionCallback(MENU_ACTION_CALLBACK_FUNC pCallback);
char * getMenuText();
//Sets the previous sibling, mostly used during menu creation to notify a new entry where it's
//previous pointer needs to point.
void setParent( menuItem * parent );

menuItem * getNextSibling();
menuItem * getPrevSibling();
menuItem * getChild();
menuItem * getParent();
//This call will call the action callback for use when the menu item is selected.
//if this menu entry has any children, the callback will not be executed.
void ExecuteCallback();

bool isBackEntry() { return (_callback == menuItem_BackCallbackFunc); }


private:

void * _userData;
char * _menuText;
menuItem* _parent;
menuItem* _child;
menuItem* _prevSibling;
menuItem* _nextSibling;
MENU_ACTION_CALLBACK_FUNC _callback;

};


#endif
