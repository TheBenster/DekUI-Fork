// SPDX-License-Identifier: GPL-2.0-or-later
import org.citra.citra_emu.overlay.ReMaskedTargetToggle;

public final class TargetToggleCheck {
    private static void check(boolean value, String message) {
        if (!value) throw new AssertionError(message);
    }
    public static void main(String[] args) {
        ReMaskedTargetToggle target = new ReMaskedTargetToggle();
        check(target.update(true, false), "Default hold presses targeting");
        check(!target.update(false, false), "Default release stops targeting");
        check(target.update(true, true), "First tap locks on");
        check(target.update(true, true), "Repeated press cannot toggle twice");
        check(target.update(false, true), "Lifting the finger keeps lock-on");
        check(!target.update(true, true), "Second tap unlocks");
        check(!target.update(false, true), "Release keeps targeting off");
        check(target.update(true, true), "Third tap locks on again");
        target.reset();
        check(!target.active(), "Menu, cancellation, or focus loss releases the latch");
        check(target.update(true, true), "A fresh tap works after reset");
        check(!target.update(false, false), "Disabling toggle mode releases targeting");
        target.reset();
        check(!target.update(false, true), "A release without a press cannot latch");
        System.out.println("Target toggle hold/tap/repeat/reset/mode-change checks passed");
    }
}
