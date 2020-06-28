/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

struct ID3D11Texture2D;
struct IDXGISwapChain;
struct ID3D11DeviceContext;
struct ID3D11Device;

namespace vr {
  class IVRSystem;
}

#include <windows.h>
#include <d3d11_1.h>

#include "mozilla/RefPtr.h"
#include "openvr.h"

// FxROutputHandler is responsible for managing resources to share a Desktop
// browser window with a Firefox Reality VR window.
// Note: this object is created on the Compositor thread, but its usage should
// only be on the RenderThread.
class FxROutputHandler final {
 public:
  explicit FxROutputHandler(uint64_t aOverlayId);

  bool TryInitialize(IDXGISwapChain* aSwapChain, ID3D11Device* aDevice);
  void UpdateOutput(ID3D11DeviceContext* aCtx);

  bool GetSize(uint32_t& aWidth, uint32_t& aHeight) const;

 private:
  // Until proper integration with VRProcess, make sure this is a static
  // singleton. It's possible for multiple output handlers to be created if
  // there is a device reset, or when multiple windows are supported.
  static vr::IVRSystem* s_pHMD;
  uint64_t mOverlayId = 0;
  RefPtr<IDXGISwapChain> mSwapChain = nullptr;
  // Cache the width/height from the last initialization for easy retrieval
  uint32_t mLastWidth = 0;
  uint32_t mLastHeight = 0;
};