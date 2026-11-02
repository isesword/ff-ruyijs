"""Trusted input is created by the existing native Interaction owner.

`glide()` and the humanized `click()` add the L6 behavioral layer: a pointer
trajectory with easing, per-point jitter and fractional landing coordinates,
dispatched through the same trusted-input pipeline.
"""
import math
import random


class InputControl:
    def dispatch_input(self, event_type, *, target="document", **init):
        """Dispatch one trusted Mouse/Pointer event and return True on delivery.

        target is a JavaScript expression evaluated in this Context. The event
        is created in that target's realm. Coordinate/flag fields use the native
        event dictionary; timeStamp reads the Context's scheduling clock. No
        layout, hit testing, or automatic compatibility mouse event is generated.
        """
        if not isinstance(event_type, str) or not isinstance(target, str):
            raise TypeError("input type and target must be strings")
        self._callbacks.guard()
        with self._lock:
            return self._request("input", type=event_type, target=target, init=init)

    # ---- humanized pointer layer ----

    _pointer = None  # last (x, y) this Context's virtual pointer occupied

    @staticmethod
    def _humanize_offset(width, height):
        """Landing offset inside the element: uniformly off-center, Gaussian edge."""
        rx = random.uniform(-0.18, 0.18)
        ry = random.uniform(-0.18, 0.18)
        return (width * rx, height * ry)

    @staticmethod
    def _trajectory(start, end, steps):
        """Eased points from start to end with per-point jitter (human hand)."""
        (x0, y0), (x1, y1) = start, end
        dx, dy = x1 - x0, y1 - y0
        dist = math.hypot(dx, dy)
        # control point perpendicular to the chord: a gentle arc, not a line
        mx, my = (x0 + x1) / 2 + (-dy / dist) * dist * 0.12 if dist else x1, \
                 (y0 + y1) / 2 + (dx / dist) * dist * 0.12 if dist else y1
        points = []
        for i in range(1, steps + 1):
            t = i / steps
            eased = t * t * (3 - 2 * t)  # smoothstep: slow start, fast middle
            bx = (1 - t) ** 2 * x0 + 2 * (1 - t) * t * mx + t * t * x1
            by = (1 - t) ** 2 * y0 + 2 * (1 - t) * t * my + t * t * y1
            jx = random.uniform(-1.2, 1.2) * (1 - abs(eased - 0.5) * 2)
            jy = random.uniform(-1.2, 1.2) * (1 - abs(eased - 0.5) * 2)
            points.append((bx + jx, by + jy))
        return points

    def glide(self, target, *, duration_ms=420, offset=None, start=None):
        """Move the virtual pointer to `target` along a humanized path.

        The pointer travels a smoothstep arc with per-point jitter, one
        pointermove/mousemove pair per ~14 ms of running event loop, and ends
        inside the element at a fractional offset from its center (offset is
        (dx, dy) in px; None picks a random in-bounds landing point). Returns
        the landing (x, y) — pass it as click(offset=...) so the button press
        lands exactly where the glide ended.
        """
        if not isinstance(target, str):
            raise TypeError("glide target must be a string")
        rect = self.eval(
            "((e)=>{const r=e.getBoundingClientRect();"
            "return {x:r.x,y:r.y,w:r.width,h:r.height}})"
            f"({target})")
        w, h = rect["w"], rect["h"]
        if offset is None:
            offset = self._humanize_offset(w, h)
        ox, oy = offset
        land = (rect["x"] + w / 2 + ox, rect["y"] + h / 2 + oy)
        start = self._pointer if self._pointer else (
            random.uniform(0, max(rect["x"] - 60, 4)), random.uniform(0, max(rect["y"] - 40, 4)))
        steps = max(10, min(40, int(duration_ms / 14)))
        self._pointer = start
        for (x, y) in self._trajectory(start, land, steps):
            mx = x - self._pointer[0]
            my = y - self._pointer[1]
            self.dispatch_input("pointermove", target="document",
                                clientX=x, clientY=y, movementX=mx, movementY=my,
                                screenX=x, screenY=y, buttons=0)
            self.dispatch_input("mousemove", target="document",
                                clientX=x, clientY=y, screenX=x, screenY=y, buttons=0)
            self._pointer = (x, y)
            self.advance(10 + random.randint(0, 6))
        self.advance(random.randint(40, 90))  # human aim pause before pressing
        self._pointer = land
        return {"land": land, "offset": offset}

    def click(self, target, *, hold_ms=80, glide=True, duration_ms=420, offset=None):
        """Click the element `target` (a JavaScript expression) as a user does.

        With glide=True (default) the pointer first travels a humanized path
        and lands off-center; the trusted down/up/click sequence then fires at
        that same fractional point. offset=(dx, dy) overrides the landing
        offset in px.
        """
        if not isinstance(target, str):
            raise TypeError("click target must be a string")
        if glide:
            landed = self.glide(target, duration_ms=duration_ms, offset=offset)
            offset = landed["offset"]  # the press fires where the glide landed
        return self._user(action="click", target=target, hold_ms=hold_ms,
                          offset_x=offset[0], offset_y=offset[1])

    def type(self, text, *, target=None, interval_ms=60):
        """Type `text` into the focused element (or focus `target` first).

        Each character is keydown, keypress, beforeinput, the insertion, input
        and keyup, with `interval_ms` of running event loop between characters.
        """
        if not isinstance(text, str) or (target is not None and not isinstance(target, str)):
            raise TypeError("type text and target must be strings")
        fields = {"text": text, "interval_ms": interval_ms}
        if target is not None:
            fields["target"] = target
        return self._user(action="type", **fields)

    def press(self, key, *, target=None):
        """Press one named key: Enter, Backspace, Tab, Escape or an arrow key.

        Enter inserts a line break in a textarea and submits from an input.
        """
        if not isinstance(key, str) or (target is not None and not isinstance(target, str)):
            raise TypeError("press key and target must be strings")
        fields = {"key": key}
        if target is not None:
            fields["target"] = target
        return self._user(action="press", **fields)

    def _user(self, **fields):
        self._callbacks.guard()
        with self._lock:
            return self._request("user", **fields)
