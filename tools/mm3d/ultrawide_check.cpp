// SPDX-License-Identifier: GPL-2.0-or-later
// c++ -std=c++20 -Isrc tools/mm3d/ultrawide_check.cpp -o /tmp/ultrawide-check
#include <cassert>
#include <iostream>
#include "mm3d/widescreen.h"
#include "mm3d/touch_hud.h"

int main() {
    namespace W=MM3D::Widescreen;
    using Rect=Common::Rectangle<unsigned>;
    W::fit_display=false;
    W::UpdateDisplayAspect(true,{100,0,1900,1080},{},2400,false);
    auto legacy=W::FitTop({100,0,1900,1080},{},2400,false);
    assert(legacy.GetWidth()==1920 && legacy.GetHeight()==1080);
    assert(legacy.left==40); // Preserve the existing slot's center, including insets.
    W::fit_display=true;
    for(unsigned width:{1920u,2340u,2400u,2520u,3840u}) {
        const Rect top{(width-1800)/2,0,(width+1800)/2,1080};
        W::UpdateDisplayAspect(true,top,{},width,false);
        const auto fitted=W::FitTop(top,{},width,false);
        assert(fitted.left==0 && fitted.right==width && fitted.top==0 && fitted.bottom==1080);
        const float aspect=float(width)/1080;
        assert(std::abs(W::CurrentAspect()-aspect)<.00001f);
        // Expanding the viewport cancels HUD clip compression in both dimensions.
        assert(std::abs(W::CurrentHorizontalScale()*width/400.f-1080.f/240.f)<.00001f);
        assert(std::abs(W::WidthExpansion()*400-width*240.f/1080)<.0001f);
        // Android glyph targets round-trip through the real vertex correction.
        for(float x:{.05f,.25f,.5f,.8f,.95f}) {
            const auto a=MM3D::TouchHud::Project(x,.6f,.04f,width,1080,0,0,width,1080,true);
            const float screen_x=((a.x-200)/200*W::CurrentHorizontalScale()*.5f+.5f)*width;
            assert(std::abs(screen_x-x*width)<.001f);
            assert(std::abs(a.y/240*1080-.6f*1080)<.001f);
            assert(std::abs(a.diameter/240*1080-.04f*width)<.001f);
        }
    }
    // A separate bottom screen keeps its own space; an overlapping inset does not.
    W::UpdateDisplayAspect(true,{0,0,1800,1080},{1800,0,2400,1080},2400,true);
    auto beside=W::FitTop({0,0,1800,1080},{1800,0,2400,1080},2400,true);
    assert(beside.right<=1800);
    W::UpdateDisplayAspect(true,{300,0,2100,1080},{1900,700,2400,1080},2400,true);
    assert(std::abs(W::CurrentAspect()-2400.f/1080)<.00001f);
    const auto main_aspect=W::CurrentAspect();
    W::UpdateDisplayAspect(false,{0,0,320,240},{},320,false);
    assert(W::CurrentAspect()==main_aspect);
    W::UpdateLayoutSupport(true,true,true,true);
    W::UpdateLayoutSupport(false,false,false,false);
    W::enabled=true;assert(W::Active());
    W::UpdateLayoutSupport(true,true,false,true);assert(!W::Active());
    W::UpdateDisplayAspect(true,{0,0,1080,2400},{},1080,false);
    assert(W::CurrentAspect()==W::Aspect);
    W::UpdateDisplayAspect(true,{},{},0,false);assert(W::CurrentAspect()==W::Aspect);
    const Rect empty{};assert(W::FitTop(empty,{},0,false)==empty);
    W::fit_display=false;W::enabled=false;
    std::cout<<"Ultrawide aspect, bounds, secondary display isolation, and touch projection passed\n";
}
