// Copyright 2026 MM3D presentation contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <algorithm>
#include <atomic>
#include <cmath>
#include "common/math_util.h"

namespace MM3D::Widescreen {
constexpr float NativeAspect = 400.f / 240.f;
constexpr float Aspect = 16.f / 9.f;
constexpr float HorizontalScale = NativeAspect / Aspect; // 15/16
inline std::atomic<bool> enabled{false};
inline std::atomic<bool> fit_display{false};
inline std::atomic<float> display_aspect{Aspect};
inline float CurrentAspect() { return fit_display.load() ? display_aspect.load() : Aspect; }
inline float CurrentHorizontalScale() { return NativeAspect / CurrentAspect(); }
inline float WidthExpansion() { return CurrentAspect() / NativeAspect; }
inline std::atomic<float> camera_zoom_out{1.f};
inline std::atomic<bool> supported_layout{false};
inline std::atomic<unsigned> corrected_draws{0};
inline unsigned frame_corrected_draws=0; // Renderer thread only.
// Guest geometry uses the main display's projection. A bottom-only or hidden
// Android display must never disable that projection for the following frame.
inline void UpdateLayoutSupport(bool primary, bool top_enabled, bool landscape, bool mono) {
    if (primary) supported_layout.store(top_enabled && landscape && mono);
}
inline bool Active() { return enabled.load() && supported_layout.load(); }
inline bool IsTopViewport(unsigned width,unsigned height) {
    // Captured MM3D top draws use 480x400, while bottom draws use 240x320.
    // Inspect guest dimensions, not cache surface offsets or resolution-scaled extents.
    return height==400 && (width==240 || width==480);
}

inline std::pair<unsigned,unsigned> TopSlot(const Common::Rectangle<unsigned>& top,
    const Common::Rectangle<unsigned>& bottom, unsigned window_width, bool bottom_enabled) {
    unsigned left=0, right=window_width;
    if(bottom_enabled && bottom.top<top.bottom && bottom.bottom>top.top) {
        if(bottom.left>=top.right) right=std::min(right,bottom.left);
        if(bottom.right<=top.left) left=std::max(left,bottom.right);
    }
    return {left,right};
}
// Only the main display can set the guest projection. A secondary display
// must not change it halfway through a frame. Use the top-screen slot so a
// separate bottom screen never becomes part of the camera's field of view.
inline void UpdateDisplayAspect(bool primary, const Common::Rectangle<unsigned>& top,
    const Common::Rectangle<unsigned>& bottom, unsigned window_width, bool bottom_enabled) {
    if(!primary) return;
    const auto [left,right]=TopSlot(top,bottom,window_width,bottom_enabled);
    const float ratio=right>left && top.GetHeight()>0 ?
        float(right-left)/float(top.GetHeight()) : Aspect;
    display_aspect.store(std::clamp(ratio,Aspect,32.f/9.f));
}

// Fit the wider top screen in its available slot. Keep separate neighboring
// screens outside the slot; an existing overlapping bottom-screen inset stays an inset.
inline Common::Rectangle<unsigned> FitTop(Common::Rectangle<unsigned> top,
    const Common::Rectangle<unsigned>& bottom, unsigned window_width, bool bottom_enabled) {
    const auto [left,right]=TopSlot(top,bottom,window_width,bottom_enabled);
    if(right<=left || !top.GetHeight()) return top;
    const float aspect=CurrentAspect();
    const float h=std::min(float(top.GetHeight()),float(right-left)/aspect);
    const unsigned height=std::max(1u,unsigned(std::lround(h)));
    const unsigned width=std::min(right-left,std::max(1u,unsigned(std::lround(h*aspect))));
    const float cx=(float(top.left)+top.right)*.5f;
    const unsigned x=unsigned(std::clamp(cx-width*.5f,float(left),float(right-width)));
    const unsigned y=top.top+(top.GetHeight()-height)/2;
    return {x,y,x+width,y+height};
}
}
