#include "menuItem.h"
#include "LCDMenu.h"


menuItem::menuItem(char * menuText, void * userData, MENU_ACTION_CALLBACK_FUNC func){

  _menuText = strdup(menuText);
  _userData = userData;
  _nextSibling = NULL;
  _prevSibling = NULL;
  _child = NULL;
  _parent = NULL;
  _callback = func;

}

void menuItem::ExecuteCallback()
{
  if( _callback != NULL )
  {
    _callback(_menuText, _userData);
  }
}

bool menuItem::addChild(menuItem* child)
{
  child->setParent( this );
  if(_child != NULL)
  {
    _child->addSibling( child );
  }
  else
  {
    _child = child;
  }
  return true;
}

void menuItem::addSibling( menuItem* sibling)
{
  sibling->setParent( _parent );
  if( _nextSibling != NULL )
  {
    _nextSibling->addSibling(sibling);
  }
  else
  {
    _nextSibling = sibling;
    sibling->setPrevSibling( this );
  }
}

void menuItem::setPrevSibling( menuItem * prevSibling)
{
  _prevSibling = prevSibling;
}

char * menuItem::getMenuText()
{
  return _menuText;
}

menuItem *menuItem::getNextSibling()
{
  return _nextSibling;
}
menuItem *menuItem::getPrevSibling()
{
  return _prevSibling;
}
menuItem *menuItem::getChild()
{
  return _child;
}
menuItem *menuItem::getParent()
{
  return _parent;
}
void menuItem::setParent( menuItem * parent)
{
  _parent = parent;
}

void menuItem_BoolTrueCallbackFunc( char * pMenuText, void * pUserData )
{
   //Serial.println("MenuEntry_BoolTrueCallbackFunc");
  *((unsigned int *)pUserData) = true;
}

void menuItem_BoolFalseCallbackFunc( char * pMenuText, void * pUserData )
{
  //Serial.println("menuItem_BoolFalseCallbackFunc");
  *((unsigned int *)pUserData) = false;
}

void menuItem_BackCallbackFunc( char * pMenuText, void * pUserData )
{
  //Serial.println("menuItem_BackCallbackFunc");
  ((LCDMenu *)pUserData)->DoMenuAction( MENU_ACTION_BACK );
}
