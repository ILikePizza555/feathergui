// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#include "fgSkin.h"
#include "fgAll.h"
#include "feathercpp.h"
#include "bss-util/cXML.h"
#include "bss-util/cTrie.h"
#include <fstream>
#include <sstream>

using namespace bss_util;

char* fgSkin_ParseFontFamily(char* s, char quote, char** context)
{
  if(!s)
  {
    s = *context;
    if(!s) // This can happen in error cases
      return 0;
    if(s[0] == ',')
      ++s;
  }
  else
    *context = 0;

  while(isspace(s[0])) ++s;
  if(!s[0]) return 0;
  char* end;
  if(s[0] == quote)
    end = strchr(++s, quote);
  else
    end = strchr(s, ' ');
  if(!end || !end[0])
    return s;
  *end = 0;
  *context = end + 1;
  return s;
}

int fgStyle_NodeEvalTransform(const cXMLNode* node, fgTransform& t)
{
  static cTrie<uint16_t, true> attr(9, "area", "center", "rotation", "left", "top", "right", "bottom", "width", "height");
  int flags = 0;
  bool hastransform = false;

  for(size_t i = 0; i < node->GetAttributes(); ++i)
  {
    const cXMLValue* a = node->GetAttribute(i);
    switch(attr[a->Name.c_str()])
    {
    case 0:
      flags &= (~(FGUNIT_LEFT_MASK | FGUNIT_TOP_MASK | FGUNIT_RIGHT_MASK | FGUNIT_BOTTOM_MASK));
      flags |= fgStyle_ParseCRect(a->String, &t.area);
      break;
    case 1:
      flags &= (~(FGUNIT_X_MASK | FGUNIT_Y_MASK));
      flags |= fgStyle_ParseCVec(a->String, &t.center);
      break;
    case 2:
      t.rotation = (FABS)a->Float;
      break;
    case 3:
      flags = (flags&(FGUNIT_LEFT_MASK)) | fgStyle_ParseCoord(a->String, 0, &t.area.left);
      break;
    case 4:
      flags = (flags&(FGUNIT_TOP_MASK)) | fgStyle_ParseCoord(a->String, 0, &t.area.top);
      break;
    case 5:
      flags = (flags&(FGUNIT_RIGHT_MASK)) | fgStyle_ParseCoord(a->String, 0, &t.area.right);
      break;
    case 6:
      flags = (flags&(FGUNIT_BOTTOM_MASK)) | fgStyle_ParseCoord(a->String, 0, &t.area.bottom);
      break;
    case 7:
      flags = (flags&(FGUNIT_RIGHT_MASK)) | fgStyle_ParseUnit(a->String, 0) | FGUNIT_RIGHT_WIDTH;
      t.area.right.abs = (FABS)a->Float;
      break;
    case 8:
      flags = (flags&(FGUNIT_BOTTOM_MASK)) | fgStyle_ParseUnit(a->String, 0) | FGUNIT_BOTTOM_HEIGHT;
      t.area.bottom.abs = (FABS)a->Float;
      break;
    default:
      continue;
    }
    hastransform = true;
  }

  if(!hastransform) return -1;
  return flags;
}

uint32_t fgStyle_ParseColor(const cXMLValue* attr)
{
  fgColor color;
  color.color = attr->Integer;
  if(attr->String.length() == 8 && attr->String[0] == '0' && attr->String[1] == 'x') // If this is true, it's a non-alpha color value
  {
    rswap(color.r, color.b);
    color.a = 255;
  }
  else
  {
    rswap(color.r, color.a);
    rswap(color.g, color.b);
  }
  return color.color;
}
void fgStyle_ParseAttributesXML(fgStyle* self, const cXMLNode* cur, int flags, fgSkinBase* root, const char* path, const char** id, fgKeyValueArray* userdata)
{
  static cTrie<uint16_t, true> t(47, "id", "min-width", "min-height", "max-width", "max-height", "skin", "alpha", "margin", "padding", "text",
    "placeholder", "color", "placecolor", "cursorcolor", "selectcolor", "hovercolor", "dragcolor", "edgecolor", "dividercolor", "font", "lineheight",
    "letterspacing", "value", "uv", "asset", "outline", "area", "center", "rotation", "left", "top", "right", "bottom", "width", "height",
    "name", "flags", "order", "inherit", "range", "splitter", "contextmenu", "reorder", "style", "xmlns:xsi", "xmlns:fg", "xsi:schemaLocation");
  static cTrie<uint16_t, true> tvalue(5, "checkbox", "curve", "progressbar", "radiobutton", "slider");
  static cTrie<uint16_t, true> tenum(5, "true", "false", "none", "checked", "indeterminate");

  int minflags = 0x8000000;
  AbsVec mindim = { -1, -1 };
  int maxflags = 0x8000000;
  AbsVec maxdim = { -1, -1 };

  for(size_t i = 0; i < cur->GetAttributes(); ++i)
  {
    const cXMLValue* attr = cur->GetAttribute(i);
    const uint16_t ID = t[attr->Name.c_str()];

    switch(ID)
    {
    case 0:
      if(id)
        *id = fgCopyText(attr->String, __FILE__, __LINE__);
      break;
    case 1:
      mindim.x = (FABS)attr->Float;
      minflags &= (~0x8000000);
      minflags |= (fgStyle_ParseUnit(attr->String, attr->String.length() + 1) << FGUNIT_X);
      break;
    case 2:
      mindim.y = (FABS)attr->Float;
      minflags &= (~0x8000000);
      minflags |= (fgStyle_ParseUnit(attr->String, attr->String.length() + 1) << FGUNIT_Y);
      break;
    case 3:
      maxdim.x = (FABS)attr->Float;
      maxflags &= (~0x8000000);
      maxflags |= (fgStyle_ParseUnit(attr->String, attr->String.length() + 1) << FGUNIT_X);
      break;
    case 4:
      maxdim.y = (FABS)attr->Float;
      maxflags &= (~0x8000000);
      maxflags |= (fgStyle_ParseUnit(attr->String, attr->String.length() + 1) << FGUNIT_Y);
      break;
    case 5: // skin
    {
      fgSkin* skin = fgSkinBase_GetSkin(root, attr->String);
      if(!skin) // Attempt loading it as an absolute XML path
        skin = fgSkinBase_LoadFileXML(root, attr->String);
      if(!skin) // relative XML path
        skin = fgSkinBase_LoadFileXML(root, cStr(path) + attr->String);
      if(!skin) // absolute UBJSON path
        skin = fgSkinBase_LoadFileUBJSON(root, attr->String);
      if(!skin) // relative UBJSON path
        skin = fgSkinBase_LoadFileUBJSON(root, cStr(path) + attr->String);
      if(skin)
        AddStyleMsg<FG_SETSKIN, void*>(self, skin);
    }
    break;
    case 6:
      AddStyleMsg<FG_SETALPHA, FABS>(self, (FABS)attr->Float);
      break;
    case 7:
    {
      AbsRect r;
      int f = fgStyle_ParseAbsRect(attr->String, &r);
      AddStyleSubMsgArg<FG_SETMARGIN, AbsRect>(self, f, &r);
      break;
    }
    case 8:
    {
      AbsRect r;
      int f = fgStyle_ParseAbsRect(attr->String, &r);
      AddStyleSubMsgArg<FG_SETPADDING, AbsRect>(self, f, &r);
      break;
    }
    case 9: // text
    case 10: // placeholder
    {
      FG_Msg msg = { 0 };
      msg.type = FG_SETTEXT;
      msg.subtype = ((ID == 9) ? FGTEXTFMT_UTF8 : FGTEXTFMT_PLACEHOLDER_UTF8);
      msg.p = const_cast<char*>(attr->String.c_str());
      fgStyle_AddStyleMsg(self, &msg, attr->String.length() + 1, 0);
      break;
    }
    case 11:
      AddStyleSubMsg<FG_SETCOLOR, ptrdiff_t>(self, FGSETCOLOR_MAIN, fgStyle_ParseColor(attr));
      break;
    case 12:
      AddStyleSubMsg<FG_SETCOLOR, ptrdiff_t>(self, FGSETCOLOR_PLACEHOLDER, fgStyle_ParseColor(attr));
      break;
    case 13:
      AddStyleSubMsg<FG_SETCOLOR, ptrdiff_t>(self, FGSETCOLOR_CURSOR, fgStyle_ParseColor(attr));
      break;
    case 14:
      AddStyleSubMsg<FG_SETCOLOR, ptrdiff_t>(self, FGSETCOLOR_SELECT, fgStyle_ParseColor(attr));
      break;
    case 15:
      AddStyleSubMsg<FG_SETCOLOR, ptrdiff_t>(self, FGSETCOLOR_HOVER, fgStyle_ParseColor(attr));
      break;
    case 16:
      AddStyleSubMsg<FG_SETCOLOR, ptrdiff_t>(self, FGSETCOLOR_DRAG, fgStyle_ParseColor(attr));
      break;
    case 17:
      AddStyleSubMsg<FG_SETCOLOR, ptrdiff_t>(self, FGSETCOLOR_EDGE, fgStyle_ParseColor(attr));
      break;
    case 18:
      AddStyleSubMsg<FG_SETCOLOR, ptrdiff_t>(self, FGSETCOLOR_DIVIDER, fgStyle_ParseColor(attr));
      break;
    case 19: // font
    {
      int size;
      short weight;
      char italic;
      const char* families = fgStyle_ParseFont(attr->String.c_str(), '\'', &size, &weight, &italic);
      _FG_FONT_DATA* index = fgSkinBase_AddFont(root, flags, families, weight, italic, size);
      if(index)
        AddStyleMsg<FG_SETFONT, void*>(self, index->font);
      break;
    }
    case 20:
      AddStyleMsg<FG_SETLINEHEIGHT, FABS>(self, (FABS)attr->Float);
      break;
    case 21:
      AddStyleMsg<FG_SETLETTERSPACING, FABS>(self, (FABS)attr->Float);
      break;
    case 22: // Value's type changes depending on what type we are
      switch(tvalue[cur->GetName()])
      {
      case 0: // Checkbox
        switch(tenum[attr->String])
        {
        case 0:
        case 3: // Both true and checked map to checked
          AddStyleSubMsg<FG_SETVALUE, ptrdiff_t>(self, FGVALUE_INT64, FGCHECKED_CHECKED);
          break;
        case 4:
          AddStyleSubMsg<FG_SETVALUE, ptrdiff_t>(self, FGVALUE_INT64, FGCHECKED_INDETERMINATE);
          break;
        default: // Everything else maps to not checked
          AddStyleSubMsg<FG_SETVALUE, ptrdiff_t>(self, FGVALUE_INT64, FGCHECKED_NONE);
          break;
        }
      case 1: // Curve
        break; // Not implemented
      case 2: // Progressbar
      case 4: // Slider
      default: // Anything else
        AddStyleSubMsg<FG_SETVALUE, FABS>(self, FGVALUE_FLOAT, (FABS)attr->Float);
        break;
      case 3: // Radiobutton
        switch(tenum[attr->String])
        {
        case 0:
        case 3: // true, checked, and indeterminate all map to true
        case 4:
          AddStyleSubMsg<FG_SETVALUE, ptrdiff_t>(self, FGVALUE_INT64, 1);
          break;
        default: // Everything else maps to not checked
          AddStyleSubMsg<FG_SETVALUE, ptrdiff_t>(self, FGVALUE_INT64, 0);
          break;
        }
      }
      break;
    case 23: // uv
    {
      CRect uv;
      int f = fgStyle_ParseCRect(attr->String, &uv);
      AddStyleSubMsgArg<FG_SETUV, CRect>(self, f, &uv);
      break;
    }
    case 24: // asset
    {
      _FG_ASSET_DATA* res = fgSkinBase_AddAssetFile(root, flags, attr->String);
      AddStyleMsg<FG_SETASSET, void*>(self, res->asset);
      break;
    }
    case 25: // outline
      AddStyleSubMsg<FG_SETOUTLINE, FABS>(self, fgStyle_ParseUnit(attr->String.c_str(), attr->String.length() + 1), (FABS)attr->Float);
      break;
    case 26: // area
    case 27: // center
    case 28: // rotation
    case 29: // left
    case 30: // top
    case 31: // right
    case 32: // bottom
    case 33: // width
    case 34: // height
    case 35: // name
    case 36: // flags
    case 37: // order
    case 38: // inherit
      break; // These are processed before we get here, so ignore them.
    case 39: // range
      AddStyleSubMsg<FG_SETRANGE, ptrdiff_t>(self, FGVALUE_INT64, attr->Integer);
      break;
    case 40: // splitter
    {
      AbsVec splitter;
      int f = fgStyle_ParseAbsVec(attr->String, &splitter);
      AddStyleSubMsg<FG_SETVALUE, float>(self, FGVALUE_FLOAT, splitter.x);
      AddStyleSubMsg<FG_SETRANGE, float>(self, FGVALUE_FLOAT, splitter.y);
      break;
    }
    case 42: // reorder
      if(!STRICMP(attr->String, "top"))
        AddStyleSubMsg<FG_SETPARENT>(self, FGSETPARENT_LAST);
      else if(!STRICMP(attr->String, "bottom"))
        AddStyleSubMsg<FG_SETPARENT>(self, FGSETPARENT_FIRST);
      break;
    case 43: // style
      AddStyleSubMsg<FG_SETSTYLE, size_t>(self, FGSETSTYLE_INDEX, fgStyle_GetAllNames(attr->String.c_str()));
      break;
    case 44:
    case 45:
    case 46: // These are all XML specific values that are only used for setting the XSD file
      break;
    case 41: // contextmenu is a recognized option, but we put it in as custom userdata anyway because we can't resolve it until the layout is resolved.
    default: // Otherwise, unrecognized attributes are set as custom userdata
      if(!userdata)
      {
        FG_Msg msg = { 0 };
        msg.type = FG_SETUSERDATA;
        msg.p = const_cast<char*>(attr->String.c_str());
        msg.p2 = const_cast<char*>(attr->Name.c_str());
        fgStyle_AddStyleMsg(self, &msg, attr->String.length() + 1, attr->Name.length() + 1);
      }
      else
      {
        size_t i = userdata->AddConstruct();
        (*userdata)[i].key = fgCopyText(attr->Name.c_str(), __FILE__, __LINE__);
        (*userdata)[i].value = fgCopyText(attr->String.c_str(), __FILE__, __LINE__);
      }
      break;
    }
  }

  if(!(minflags & 0x8000000))
    AddStyleSubMsg<FG_SETDIM, FABS, FABS>(self, minflags | FGDIM_MIN, mindim.x, mindim.y);
  if(!(maxflags & 0x8000000))
    AddStyleSubMsg<FG_SETDIM, FABS, FABS>(self, maxflags | FGDIM_MAX, maxdim.x, maxdim.y);
}


fgFlag fgSkinBase_ParseFlagsFromString(const char* s, fgFlag* remove)
{
  static cTrie<uint16_t, true> t(54, "BACKGROUND", "NOCLIP", "IGNORE", "HIDDEN", "SILENT", "EXPANDX", "EXPANDY", "DISABLE", "SNAPX", "SNAPY", "HIDEH",
    "HIDEV", "SHOWH", "SHOWV", "IGNOREMARGINEDGEX", "IGNOREMARGINEDGEY", "TILEX", "TILEY", "DISTRIBUTEX", "DISTRIBUTEY", "SINGLESELECT", "MULTISELECT",
    "DRAGGABLE", "HIDEH", "HIDEV", "SHOWH", "SHOWV", "CHARWRAP", "WORDWRAP", "ELLIPSES", "RTL", "RIGHTALIGN", "CENTER", "SUBPIXEL", "ACTION",
    "SINGLELINE", "NOFOCUS", "RECT", "CIRCLE", "LINE", "QUADRATIC", "CUBIC", "BSPLINE", "MINIMIZABLE", "MAXIMIZABLE", "RESIZABLE",
    "NOTITLEBAR", "NOBORDER", "EXPAND", "SNAP", "TILE", "DISTRIBUTE", "TRIANGLE", "IGNOREMARGINEDGE");

  if(!s)
    return 0;
  fgFlag out = 0;
  const char* n;
  while(s)
  {
    n = strchr(s, '|');
    bool del = s[0] == '-';
    if(del)
      ++s;
    uint16_t i = !n ? t[s] : t.Get(s, n - s);
    fgFlag f = 0;
    if(i < t.Length())
    {
      switch(i)
      {
      default:
        f = (1 << i);
        break;
      case 21: f = FGLIST_MULTISELECT; break; // We can't rely on the default method here because MULTISELECT is actually two seperate flags
      case 22: f = FGLIST_DRAGGABLE; break;
      case 23: f = FGSCROLLBAR_HIDEH; break;
      case 24: f = FGSCROLLBAR_HIDEV; break;
      case 25: f = FGSCROLLBAR_SHOWH; break;
      case 26: f = FGSCROLLBAR_SHOWV; break;
      case 27: f = FGTEXT_CHARWRAP; break;
      case 28: f = FGTEXT_WORDWRAP; break;
      case 29: f = FGTEXT_ELLIPSES; break;
      case 30: f = FGTEXT_RTL; break;
      case 31: f = FGTEXT_RIGHTALIGN;  break;
      case 32: f = FGTEXT_CENTER;  break;
      case 33: f = FGTEXT_SUBPIXEL;  break;
      case 34: f = FGTEXTBOX_ACTION;  break;
      case 35: f = FGTEXTBOX_SINGLELINE;  break;
      case 36: f = FGBUTTON_NOFOCUS;  break;
      case 37: f = FGRESOURCE_RECT;  break;
      case 38: f = FGRESOURCE_CIRCLE;  break;
      case 39: f = FGCURVE_LINE;  break;
      case 40: f = FGCURVE_QUADRATIC;  break;
      case 41: f = FGCURVE_CUBIC;  break;
      case 42: f = FGCURVE_BSPLINE;  break;
      case 43: f = FGWINDOW_MINIMIZABLE;  break;
      case 44: f = FGWINDOW_MAXIMIZABLE;  break;
      case 45: f = FGWINDOW_RESIZABLE;  break;
      case 46: f = FGWINDOW_NOTITLEBAR;  break;
      case 47: f = FGWINDOW_NOBORDER;  break;
      case 48: f = FGELEMENT_EXPAND;  break;
      case 49: f = FGELEMENT_SNAP;  break;
      case 50: f = FGBOX_TILE;  break;
      case 51: f = FGBOX_DISTRIBUTEX | FGBOX_DISTRIBUTEY;  break;
      case 52: f = FGRESOURCE_TRIANGLE; break;
      case 53: f = FGBOX_IGNOREMARGINEDGE; break;
      }
    }

    if(del && remove != 0)
      *remove |= f;
    else
      out |= f;
    s = !n ? 0 : n + 1;
  }

  return out;
}

fgSkin* fgSkinBase_GetInherit(fgSkinBase* self, const char* inherit)
{
  if(!inherit) return 0;
  fgSkin* r = 0;
  while(!r && self != 0)
  {
    r = fgSkinBase_GetSkin(self, inherit);
    self = self->parent;
  }
  return r;
}

// Styles parse flags differently - the attributes use the parent flags for resource/font loading, then creates add and remove flag messages
void fgSkinBase_ParseStyleNodeXML(fgSkinBase* self, fgStyle* style, fgFlag rootflags, const cXMLNode* cur)
{
  fgStyle_ParseAttributesXML(style, cur, rootflags, self, 0, 0, 0);
  fgFlag remove = 0;
  fgFlag add = fgSkinBase_ParseFlagsFromString(cur->GetAttributeString("flags"), &remove);
  if(remove)
    AddStyleMsg<FG_SETFLAG, ptrdiff_t, size_t>(style, remove, 0);
  if(add)
    AddStyleMsg<FG_SETFLAG, ptrdiff_t, size_t>(style, add, 1);
}

void fgSkinBase_ParseSubNodeXML(fgSkinTree* tree, fgStyle* style, fgSkinBase* root, fgSkin* skin, const cXMLNode* cur)
{
  fgFlag remove = 0;
  fgFlag rootflags = fgSkinBase_ParseFlagsFromString(cur->GetAttributeString("flags"), &remove);
  if(remove)
    AddStyleMsg<FG_SETFLAG, ptrdiff_t, size_t>(style, remove, 0);
  if(rootflags)
    AddStyleMsg<FG_SETFLAG, ptrdiff_t, size_t>(style, rootflags, 1);

  fgTransform ts = { 0 };
  int tsunits = fgStyle_NodeEvalTransform(cur, ts);
  if(tsunits != -1)
    AddStyleSubMsgArg<FG_SETTRANSFORM, fgTransform>(style, tsunits, &ts);

  fgStyle_ParseAttributesXML(style, cur, rootflags, root, 0, 0, 0);
  if(skin)
    skin->inherit = fgSkinBase_GetInherit(&skin->base, cur->GetAttributeString("inherit"));

  for(size_t i = 0; i < cur->GetNodes(); ++i)
  {
    const cXMLNode* node = cur->GetNode(i);
    if(skin != nullptr && !STRICMP(node->GetName(), "skin"))
      fgSkinBase_ParseNodeXML(&skin->base, node);
    else if(!STRICMP(node->GetName(), "style"))
      fgSkinBase_ParseStyleNodeXML(root, fgSkinTree_GetStyle(tree, fgSkinTree_AddStyle(tree, node->GetAttributeString("name"))), rootflags, node);
    else
    {
      fgTransform transform = { 0 };
      short units = fgStyle_NodeEvalTransform(node, transform);
      fgFlag rmflags = 0;
      fgFlag flags = fgSkinBase_ParseFlagsFromString(node->GetAttributeString("flags"), &rmflags);
      flags = (flags | fgGetTypeFlags(node->GetName()))&(~rmflags);
      fgSkinLayout* child = fgSkinTree_GetChild(tree, (FG_UINT)fgSkinTree_AddChild(tree, node->GetName(), flags, &transform, units, (int)node->GetAttributeInt("order")));
      fgStyle_ParseAttributesXML(&child->layout.style, node, flags, root, 0, 0, 0);
      _sendsubmsg<FG_SETSTYLE, void*, size_t>(child->instance, FGSETSTYLE_POINTER, (void*)&child->layout.style, ~0);
      fgSkinBase_ParseSubNodeXML(&child->tree, &child->layout.style, root, 0, node);
    }
  }
}

fgSkin* fgSkinBase_ParseNodeXML(fgSkinBase* self, const cXMLNode* root)
{
  const char* id = root->GetAttributeString("id");
  if(!id)
    id = root->GetAttributeString("name");
  if(!id)
    return 0;
  fgSkin* s = fgSkinBase_AddSkin(self, id);
  fgSkinBase_ParseSubNodeXML(&s->tree, &s->style, &s->base, s, root);
  return s;
}

fgSkin* fgSkinBase_ParseStreamXML(fgSkinBase* self, std::istream& s)
{
  cXML xml(s);
  size_t index = 0;
  const cXMLNode* root;
  fgSkin* ret = 0;

  while(root = xml.GetNode(index++))
  {
    if(!STRICMP(root->GetName(), "fg:skin"))
      ret = fgSkinBase_ParseNodeXML(self, root);
  }

  return ret;
}

fgSkin* fgSkinBase_LoadFileXML(fgSkinBase* self, const char* file)
{
  std::ifstream s(file, std::ios_base::in | std::ios_base::binary);
  return fgSkinBase_ParseStreamXML(self, s);
}
fgSkin* fgSkinBase_LoadXML(fgSkinBase* self, const char* data, FG_UINT length)
{
  std::istringstream s(std::string(data, length));
  return fgSkinBase_ParseStreamXML(self, s);
}