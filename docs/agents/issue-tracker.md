# Issue tracker: GitHub

Issues and specs for this repo live as GitHub issues on
**`Elismar13/thushimas-car`**. Use the `gh` CLI for all operations.

> **This repo has two git remotes** — `origin` points at GitLab
> (`gitlab.com:elismar.silva/thushimas-car`) and `github` points at GitHub
> (`github.com/Elismar13/thushimas-car`). `gh` infers the repository from the
> remotes and can pick the wrong one. **Always pass `--repo Elismar13/thushimas-car`
> explicitly.** GitLab is not the tracker: `glab` is not installed on this machine.

## Conventions

- **Create an issue**: `gh issue create --repo Elismar13/thushimas-car --title "..." --body "..."`. Use a heredoc for multi-line bodies.
- **Read an issue**: `gh issue view <number> --repo Elismar13/thushimas-car --comments`.
- **List issues**: `gh issue list --repo Elismar13/thushimas-car --state open --json number,title,body,labels --jq '[.[] | {number, title, labels: [.labels[].name]}]'` with `--label` and `--state` filters.
- **Comment on an issue**: `gh issue comment <number> --repo Elismar13/thushimas-car --body "..."`
- **Apply / remove labels**: `gh issue edit <number> --repo Elismar13/thushimas-car --add-label "..."` / `--remove-label "..."`
- **Close**: `gh issue close <number> --repo Elismar13/thushimas-car --comment "..."`

## Pull requests as a triage surface

**PRs as a request surface: no.** _(Set to `yes` if this repo treats external PRs as feature requests; `/triage` reads this flag.)_

## When a skill says "publish to the issue tracker"

Create a GitHub issue.

## When a skill says "fetch the relevant ticket"

Run `gh issue view <number> --repo Elismar13/thushimas-car --comments`.

## Blocking dependencies

GitHub's **native issue dependencies** are the canonical representation. Add an edge
with `gh api --method POST repos/Elismar13/thushimas-car/issues/<child>/dependencies/blocked_by -F issue_id=<blocker-db-id>`, where `issue_id` is the blocker's numeric **database id**
(`gh api repos/Elismar13/thushimas-car/issues/<n> --jq .id`, _not_ the `#number`).

Where dependencies are unavailable, fall back to a `Blocked by: #<n>, #<n>` line at the
top of the child body. A ticket is unblocked when every blocker is closed.
