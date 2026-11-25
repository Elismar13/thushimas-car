# Pivot to a phone-controlled car

The repository began as an autonomous-navigation research platform: an 11-phase roadmap
running from throttle-and-steering control through mapping, localization and path
planning, with a rotating dual-sonar head feeding 360° scans to a PC over MQTT. What
actually existed was a steering servo, a sonar head, an OLED and a desktop viewer — no
motor driver, no E-stop, no obstacle interlock. The car had never driven.

On branch `feat/phone-controlled-car` we drop the research agenda and build the missing
half instead: motor control, safety interlocks, and a phone control surface served by
the ESP32 over HTTP. The `main` line is left intact as the research line rather than
being rewritten, so the navigation work survives and the two lines are not kept in sync.

The phone replaces the PC, the broker and the desktop GUI as the only client, which is
what removes the MQTT stack from the critical path. The trade-off accepted in exchange
is that this branch gives up 360° sensing, mapping and navigation entirely, and that
the on-car display is dropped in favour of the phone.

Consequences: the 360° scan, the MQTT client, the OLED and the `tools/` viewer are
deleted on this branch; `docs/decisions.md` gains a Branch Overrides table recording
which `main`-line decisions this branch replaces.
