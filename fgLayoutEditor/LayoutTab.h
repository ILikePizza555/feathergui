// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#ifndef __LAYOUT_TAB_H__
#define __LAYOUT_TAB_H__

#include "EditorBase.h"
#include "bss-util/Hash.h"

class LayoutTab
{
public:
  LayoutTab();
  ~LayoutTab();
  void Init(EditorBase* base);
  void OpenLayout(fgLayout* layout);
  void AddProp(const char* name, FG_UINT id, const char* type = "textbox");
  void Clear();
  void Link(fgElement* e, fgClassLayout* layout);
  inline void ClearLinks() { _elementmap.Clear(); _layoutmap.Clear(); }
  fgLayout* FindParentLayout(fgElement* treeitem);
  inline fgElement* GetTreeItem(fgClassLayout* p) { return _treemap[p]; }
  inline fgClassLayout* GetClassLayout(fgElement* p) { return _elementmap[p]; }
  void RemoveElement(fgElement* e);
  void InsertElement(fgElement* e, const char* type, bool insert, fgTransform* tf);
  inline fgElement* GetSelected() { return _selected; }

  void MenuInsert(struct _FG_ELEMENT*, const FG_Msg*);
  void MenuAdd(struct _FG_ELEMENT*, const FG_Msg*);
  void MenuContext(struct _FG_ELEMENT*, const FG_Msg*);

  fgElement* _contextmenulayout;

protected:
  void _openSublayout(fgElement* root, fgLayout* parent);
  void _openLayout(fgElement* root, const fgVectorClassLayout& layout);
  void _treeItemOnFocus(struct _FG_ELEMENT*, const FG_Msg*);
  void _treeItemOnHover(struct _FG_ELEMENT*, const FG_Msg*);
  void _textboxOnAction(struct _FG_ELEMENT* e, const FG_Msg* msg);
  void _flagOnAction(struct _FG_ELEMENT* e, const FG_Msg* msg);
  void _editboxOnAction(struct _FG_ELEMENT* e, const FG_Msg* msg);
  void _treeviewOnMouseDown(struct _FG_ELEMENT* e, const FG_Msg* msg);

  // Message handler for the edit box
  static size_t _treeviewMessage(fgTreeview* self, const FG_Msg* msg);
  static void _addLayoutSkins(fgSkinBase* layout, fgElement* target);

  fgTextbox _editbox;
  EditorBase* _base;
  fgTreeview* _layoutview;
  fgGrid* _layoutprops;
  fgElement* _contextmenu;
  fgElement* _selected; // Current selected item in the tree, if applicable
  bss::Hash<fgElement*, fgClassLayout*> _elementmap; // Map of elements to their corresponding class layout
  bss::Hash<fgClassLayout*, fgElement*> _layoutmap; // reverse hash of the above
  bss::Hash<fgClassLayout*, fgElement*> _treemap; // Map of class layout to the corresponding treeitem
  std::function<void(fgElement*, const char*)> _propafter;
};

#endif
