/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <cstdint>
#include <vector>
#include "mozilla/Atomics.h"
#include "nsAString.h"
#include "windows.h"
#include "openvr.h"

class nsPIDOMWindowOuter;
class nsWindow;
class nsIWidget;
namespace vr {
  class IVRSystem;
};

typedef std::vector<vr::VREvent_t> VREventVector;

// FxRWindowManager is a singleton that is responsible for tracking all of
// the top-level windows created for Firefox Reality on Desktop. Only a
// single window is initially supported.
class FxRWindowManager final {
 // TODO: make this a class, and move methods from the manager that only
 // interact with struct members to this class (i.e., some of the static
 // functions that take an FxRWindow
 private:
  struct FxRWindow {
    // Note: mWidget takes a full reference
    nsIWidget* mWidget;
    nsPIDOMWindowOuter* mWindow;
    vr::VROverlayHandle_t mOverlayHandle;
    vr::VROverlayHandle_t mOverlayThumbnailHandle;
    HWND mHwndWidget;

    // Works with Collect/ProcessOverlayEvents to transfer OpenVR input events
    // from the background thread to the main thread. Consider using a queue?
    VREventVector mEventsVector;
    CRITICAL_SECTION mEventsCritSec;

    // OpenVR scroll event doesn't provide the position of the controller on the
    // overlay, so keep track of the last-known position to use with the scroll
    // event
    POINT mLastMousePt;
    RECT mOverlaySizeRec;
  };

 public:
  static FxRWindowManager* GetInstance();
  ~FxRWindowManager();

  bool VRinit();
  void SetRenderPid(uint64_t aOverlayId, uint32_t aPid);

  bool AddWindow(nsPIDOMWindowOuter* aWindow);
  void RemoveWindow(uint64_t aOverlayId);
  bool IsFxRWindow(uint64_t aOuterWindowID);
  bool IsFxRWindow(const nsWindow* aWindow) const;

  void ShowVirtualKeyboard(uint64_t aOverlayId);
  void HideVirtualKeyboard();

  void OnWebXRPresentationChange(uint64_t aOuterWindowID, bool isPresenting);
  void SetPlayMediaState(const nsAString& aState);
  void SetProjectionMode(const nsAString& aMode);

  void ProcessOverlayEvents(nsWindow* window);

 private:
  static void InitWindow(FxRWindow& newWindow, nsPIDOMWindowOuter* aWindow);
  static void CleanupWindow(FxRWindow& fxrWindow);
  FxRWindow& GetFxrWindowFromWidget(nsIWidget* widget);  
  bool CreateOverlayForWindow(FxRWindow& newWindow, char* name, float width);

  vr::VROverlayError SetupOverlayInput(vr::VROverlayHandle_t overlayId);
  static DWORD OverlayInputPump(_In_ LPVOID lpParameter);
  void CollectOverlayEvents(FxRWindow& fxrWindow);

  void ToggleMedia();
  void EnsureTransportControls();
  void HideTransportControls();

  FxRWindowManager();

  // Members for OpenVR
  vr::IVRSystem * mVrApp;
  int32_t mDxgiAdapterIndex;

  // Members for Input
  mozilla::Atomic<bool> mIsOverlayPumpActive;
  HANDLE mOverlayPumpThread;

  // Only a single window is supported for tracking. Support for multiple
  // windows will require a data structure to collect windows as they are
  // created.
  FxRWindow mFxRWindow;
  FxRWindow mTransportWindow;
};