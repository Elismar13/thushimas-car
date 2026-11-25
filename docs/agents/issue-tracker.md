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

GitHub's **native issue dependencies** are the canonical representation, and they work
on this repo — use them rather than relying on the `Blocked by:` body line.

**The API form is easy to get backwards.** The **path** takes the child's **issue
number**; the body takes the blocker's **numeric database id**:

```bash
# <number> in the path, <db id> in the body
gh api --method POST repos/Elismar13/thushimas-car/issues/<child-number>/dependencies/blocked_by \
  -F issue_id=<blocker-db-id>
```

Get the db id with `gh api repos/Elismar13/thushimas-car/issues/<n> --jq .id` — it is
**not** the `#number`. Passing a number as `issue_id` returns `404 Not Found`.

To read the edges back:

```bash
gh api repos/Elismar13/thushimas-car/issues/<n>/dependencies/blocked_by --jq '[.[].number]'
```

To remove an edge:

```bash
gh api --method DELETE repos/Elismar13/thushimas-car/issues/<child-number>/dependencies/blocked_by/<blocker-db-id>
```

A ticket is unblocked when every blocker is closed. The unblocked count is
`.issue_dependencies_summary.blocked_by` on the issue.
