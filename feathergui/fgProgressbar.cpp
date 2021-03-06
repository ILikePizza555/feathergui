// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#include "fgProgressbar.h"
#include "fgSkin.h"
#include "feathercpp.h"

void fgProgressbar_Init(fgProgressbar* BSS_RESTRICT self, fgElement* BSS_RESTRICT parent, fgElement* BSS_RESTRICT next, const char* name, fgFlag flags, const fgTransform* transform, fgMsgType units)
{
  fgElement_InternalSetup(*self, parent, next, name, flags, transform, units, (fgDestroy)&fgProgressbar_Destroy, (fgMessage)&fgProgressbar_Message);
}

void fgProgressbar_Destroy(fgProgressbar* self)
{
  self->control->message = (fgMessage)fgControl_Message;
  fgControl_Destroy(&self->control);
}

size_t fgProgressbar_Message(fgProgressbar* self, const FG_Msg* msg)
{
  static const fgTransform BAR_ELEMENT = { 0,0,0,0,0,0,0,1.0,0,0,0,0,0 };

  assert(self != 0 && msg != 0);
  switch(msg->type)
  {
  case FG_CONSTRUCT:
    fgControl_Message(&self->control, msg);
    fgElement_Init(&self->bar, *self, 0, "Progressbar$bar", FGELEMENT_BACKGROUND | FGELEMENT_IGNORE | FGFLAGS_INTERNAL, &BAR_ELEMENT, 0);
    fgText_Init(&self->text, *self, 0, "Progressbar$text", FGELEMENT_EXPAND | FGELEMENT_IGNORE | FGFLAGS_INTERNAL, &fgTransform_CENTER, 0);
    self->value = 0.0f;
    return FG_ACCEPT;
  case FG_CLONE:
    if(msg->e)
    {
      fgProgressbar* hold = reinterpret_cast<fgProgressbar*>(msg->e);
      self->bar.Clone(&hold->bar);
      self->text->Clone(hold->text);
      fgControl_Message(&self->control, msg);
      hold->value = self->value;
      _sendmsg<FG_ADDCHILD, fgElement*>(msg->e, &hold->bar);
      _sendmsg<FG_ADDCHILD, fgElement*>(msg->e, hold->text);
    }
    return sizeof(fgProgressbar);
  case FG_SETVALUE:
    if(msg->subtype <= FGVALUE_FLOAT)
    {
      float value = (msg->subtype == FGVALUE_INT64) ? (float)msg->i : msg->f;
      if(value != self->value)
      {
        self->value = value;
        CRect area = self->bar.transform.area;
        area.right.rel = self->value;
        _sendmsg<FG_SETAREA, void*>(&self->bar, &area);
      }
      return FG_ACCEPT;
    }
    fgLog(FGLOG_INFO, "%s set invalid value type: %hu", fgGetFullName(*self).c_str(), msg->subtype);
    return 0;
  case FG_GETVALUE:
    if(!msg->subtype || msg->subtype == FGVALUE_FLOAT)
      return *reinterpret_cast<size_t*>(&self->value);
    if(msg->subtype == FGVALUE_INT64)
      return (size_t)self->value;
    fgLog(FGLOG_INFO, "%s requested invalid value type: %hu", fgGetFullName(*self).c_str(), msg->subtype);
    return 0;
  case FG_CLEAR:
    fgSendMessage(self->text, msg);
    break;
  case FG_SETTEXT:
  case FG_SETFONT:
  case FG_SETLINEHEIGHT:
  case FG_SETLETTERSPACING:
  case FG_SETCOLOR:
  case FG_GETTEXT:
  case FG_GETFONT:
  case FG_GETLINEHEIGHT:
  case FG_GETLETTERSPACING:
  case FG_GETCOLOR:
    return fgSendMessage(self->text, msg);
  case FG_GETCLASSNAME:
    return (size_t)"Progressbar";
  }
  return fgControl_Message(&self->control, msg);
}
