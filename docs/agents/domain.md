# Domain Docs

How the engineering skills should consume this repo's domain documentation when
exploring the codebase.

## Layout: single-context

One `CONTEXT.md` at the repo root, ADRs in `docs/adr/`.

```
/
├── CONTEXT.md
└── docs/
    └── adr/
```

## Before exploring, read these

- **`CONTEXT.md`** at the repo root — the glossary of this project's domain
  vocabulary. Read it before writing any code, issue, spec or test name.
- **`docs/adr/`** — read the ADRs that touch the area you're about to work in.

If any of these files don't exist, **proceed silently**. Don't flag their absence;
don't suggest creating them upfront. The `/domain-modeling` skill (reached via
`/grill-with-docs` and `/improve-codebase-architecture`) creates them lazily when
terms or decisions actually get resolved.

## Use the glossary's vocabulary

When your output names a domain concept (in an issue title, a refactor proposal, a
hypothesis, a test name), use the term as defined in `CONTEXT.md`. Don't drift to
synonyms the glossary explicitly avoids.

Key terms for the current branch: **Car**, **Throttle**, **Requested throttle**,
**Actual throttle**, **Motor duty**, **Steering angle**, **Straight**,
**Obstacle avoidance**, **Interlock**, **Stop threshold**, **E-stop**,
**Latched stop**, **Armed**, **Blind spot**, **Battery state**, **Low battery**,
**Critical battery**, **Charging**, **Car link**, **Heartbeat**, **Control intent**,
**Phone**.

If the concept you need isn't in the glossary yet, that's a signal: either you're
inventing language the project doesn't use (reconsider) or there's a real gap (note it
for `/domain-modeling`).

## Flag ADR conflicts

If your output contradicts an existing ADR, surface it explicitly rather than
silently overriding:

> _Contradicts ADR-0007 (event-sourced orders), but worth reopening because…_

ADR-0001 records the pivot from the autonomous-navigation research platform to a
phone-controlled car. It governs the `feat/phone-controlled-car` branch; the `main`
line still follows the original roadmap.
