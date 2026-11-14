// SPDX-License-Identifier: GPL-2.0-or-later
package org.citra.citra_emu.overlay;

/** Touch-only targeting latch; reset whenever the input surface loses ownership. */
public final class ReMaskedTargetToggle {
    private boolean pressed, active, toggleMode;

    public boolean update(boolean down, boolean toggle) {
        if (toggle != toggleMode) {
            reset();
            toggleMode = toggle;
        }
        if (toggleMode) {
            if (down && !pressed) active = !active;
        } else {
            active = down;
        }
        pressed = down;
        return active;
    }

    public boolean active() { return active; }

    public void reset() {
        pressed = active = toggleMode = false;
    }
}
