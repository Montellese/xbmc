#pragma once
/*
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "input/touch/ITouchInputHandler.h"
#include "input/touch/TouchTypes.h"
#include "threads/CriticalSection.h"
#include "threads/Timer.h"

#define TOUCH_MAX_POINTERS  2

/*!
 * \ingroup touch_generic
 * \brief Generic implementation of ITouchInputHandler to handle low level (raw)
 *        touch events and translate them into touch actions which are passed
 *        on to the registered ITouchActionHandler implementation.
 *
 * The generic implementation supports single a double touch and hold
 * actions and basic gesture recognition for panning, swiping, pinching/zooming
 * and rotating.
 *
 * \sa ITouchInputHandler
 */
class CGenericTouchInputHandler : public ITouchInputHandler, private ITimerCallback
{
public:
  /*!
   \brief Get an instance of the touch input manager
   */
  static CGenericTouchInputHandler &Get();

  // implementation of ITouchInputHandler
  virtual bool HandleTouchInput(TouchInput event, float x, float y, int64_t time, int32_t pointer = 0, float size = 0.0f);
  virtual bool UpdateTouchPointer(int32_t pointer, float x, float y, int64_t time, float size = 0.0f);

private:
  // private construction, and no assignements; use the provided singleton methods
  CGenericTouchInputHandler();
  CGenericTouchInputHandler(const CGenericTouchInputHandler&);
  CGenericTouchInputHandler const& operator=(CGenericTouchInputHandler const&);
  virtual ~CGenericTouchInputHandler();

  typedef enum {
    TouchGestureUnknown = 0,
    // only primary pointer active but stationary so far
    TouchGestureSingleTouch,
    // primary pointer active but stationary for a certain time
    TouchGestureSingleTouchHold,
    // primary pointer moving
    TouchGesturePan,
    // at least two pointers active but stationary so far
    TouchGestureMultiTouchStart,
    // at least two pointers active but stationary for a certain time
    TouchGestureMultiTouchHold,
    // at least two pointers active and moving
    TouchGestureMultiTouch,
    // all but primary pointer have been lifted
    TouchGestureMultiTouchDone
  } TouchGestureState;

  // implementation of ITimerCallback
  virtual void OnTimeout();

  void saveLastTouch();

  void handleMultiTouchGesture();
  void handleZoomPinch();
  void handleRotation();

  void setGestureState(TouchGestureState gestureState) { m_gestureStateOld = m_gestureState; m_gestureState = gestureState; }

  CCriticalSection m_critical;
  CTimer *m_holdTimer;
  Pointer m_pointers[TOUCH_MAX_POINTERS];
  float m_fRotateAngle;

  TouchGestureState m_gestureState;
  TouchGestureState m_gestureStateOld;
};
