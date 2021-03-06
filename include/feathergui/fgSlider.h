// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#ifndef __FG_SLIDER_H__
#define __FG_SLIDER_H__

#include "fgControl.h"

#ifdef  __cplusplus
extern "C" {
#endif

// A slider can be dragged along either a smooth value or between integer increments
typedef struct _FG_SLIDER {
  fgControl control;
  fgElement slider; //Slider object centered around the current value
  size_t range; // Represents a range from [0,range], INCLUSIVE. Set with FG_SETSTATE and a second argument set to 1
  size_t value;
#ifdef  __cplusplus
  inline operator fgElement*() { return &control.element; }
  inline fgElement* operator->() { return operator fgElement*(); }
#endif
} fgSlider;

FG_EXTERN void fgSlider_Init(fgSlider* BSS_RESTRICT self, fgElement* BSS_RESTRICT parent, fgElement* BSS_RESTRICT next, const char* name, fgFlag flags, const fgTransform* transform, fgMsgType units);
FG_EXTERN void fgSlider_Destroy(fgSlider* self);
FG_EXTERN size_t fgSlider_Message(fgSlider* self, const FG_Msg* msg);

#ifdef  __cplusplus
}
#endif

#endif