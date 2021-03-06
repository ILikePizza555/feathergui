// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#ifndef __FG_STYLE_H__
#define __FG_STYLE_H__

#include "feathergui.h"

#ifdef  __cplusplus
extern "C" {
#endif

  typedef struct _FG_STYLE_MSG
  {
    FG_Msg msg;
    struct _FG_STYLE_MSG* next;
    unsigned int sz;
  } fgStyleMsg;

  typedef struct _FG_STYLE
  {
    fgStyleMsg* styles;

#ifdef  __cplusplus
    FG_DLLEXPORT fgStyleMsg* AddStyleMsg(const FG_Msg* msg, unsigned int arg1size = 0, unsigned int arg2size = 0);
    FG_DLLEXPORT void RemoveStyleMsg(fgStyleMsg* msg);
#endif
  } fgStyle;

  FG_EXTERN void fgStyle_Init(fgStyle* self);
  FG_EXTERN void fgStyle_InitCopy(fgStyle* self, const fgStyle* from);
  FG_EXTERN void fgStyle_Destroy(fgStyle* self);
  FG_EXTERN fgStyleIndex fgStyle_GetName(const char* name);
  FG_EXTERN fgStyleIndex fgStyle_GetAllNames(const char* names);
  FG_EXTERN const char* fgStyle_GetMapIndex(fgStyleIndex index);
  FG_EXTERN fgStyleIndex fgStyle_GetIndexGroups(fgStyleIndex index);
  FG_EXTERN fgStyleIndex fgStyle_AddGroup(fgStyleIndex group); // Returns 0 on success, otherwise it returns the group that collides with this one.
  FG_EXTERN fgStyleIndex fgStyle_AddGroupNames(size_t n, ...);

  FG_EXTERN fgStyleMsg* fgStyle_AddStyleMsg(fgStyle* self, const FG_Msg* msg, unsigned int arg1size, unsigned int arg2size);
  FG_EXTERN fgStyleMsg* fgStyle_CloneStyleMsg(const fgStyleMsg* self);
  FG_EXTERN void fgStyle_RemoveStyleMsg(fgStyle* self, fgStyleMsg* msg);

  FG_EXTERN const char* fgStyle_ParseFont(const char* font, char quote, int* size, short* weight, char* italic);
  FG_EXTERN int fgStyle_ParseUnit(const char* str, size_t len);
  FG_EXTERN int fgStyle_ParseCoord(const char* attribute, size_t len, Coord* coord);
  FG_EXTERN int fgStyle_ParseAbsRect(const char* attribute, AbsRect* r);
  FG_EXTERN int fgStyle_ParseAbsVec(const char* attribute, AbsVec* r);
  FG_EXTERN int fgStyle_ParseCRect(const char* attribute, CRect* r);
  FG_EXTERN int fgStyle_ParseCVec(const char* attribute, CVec* v);

  FG_EXTERN size_t fgStyle_WriteCVec(char* buffer, size_t sz, const CVec* vec, fgMsgType units);
  FG_EXTERN size_t fgStyle_WriteAbsRect(char* buffer, size_t sz, const AbsRect* r, fgMsgType units);
  FG_EXTERN size_t fgStyle_WriteAbsVec(char* buffer, size_t sz, const AbsVec* r, fgMsgType units);
  FG_EXTERN size_t fgStyle_WriteCRect(char* buffer, size_t sz, const CRect* r, fgMsgType units);
  FG_EXTERN size_t fgStyle_WriteFlagsIterate(char* buffer, size_t sz, const char* type, const char* divider, fgFlag flags, char remove);
#ifdef  __cplusplus
}

template<FG_MSGTYPE type, typename... Args>
inline fgStyleMsg* AddStyleMsg(fgStyle* style, Args... args)
{
  FG_Msg msg = { 0 };
  msg.type = type;
  fgSendMsgCall<1, Args...>::F(msg, args...);
  return fgStyle_AddStyleMsg(style, &msg, 0, 0);
}
template<FG_MSGTYPE type, typename... Args>
inline fgStyleMsg* AddStyleSubMsg(fgStyle* style, fgMsgType sub, Args... args)
{
  FG_Msg msg = { 0 };
  msg.type = type;
  msg.subtype = sub;
  fgSendMsgCall<1, Args...>::F(msg, args...);
  return fgStyle_AddStyleMsg(style, &msg, 0, 0);
}
template<FG_MSGTYPE type, typename Arg, typename... Args>
inline fgStyleMsg* AddStyleMsgArg(fgStyle* style, const Arg* arg, Args... args)
{
  FG_Msg msg = { 0 };
  msg.type = type;
  msg.p = (void*)arg;
  fgSendMsgCall<2, Args...>::F(msg, args...);
  return fgStyle_AddStyleMsg(style, &msg, sizeof(Arg), 0);
}
template<FG_MSGTYPE type, typename Arg, typename... Args>
inline fgStyleMsg* AddStyleSubMsgArg(fgStyle* style, fgMsgType sub, const Arg* arg, Args... args)
{
  FG_Msg msg = { 0 };
  msg.type = type;
  msg.subtype = sub;
  msg.p = (void*)arg;
  fgSendMsgCall<2, Args...>::F(msg, args...);
  return fgStyle_AddStyleMsg(style, &msg, sizeof(Arg), 0);
}
#endif

#endif