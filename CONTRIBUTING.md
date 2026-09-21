# Development Workflow & Contribution Guidelines

This document defines the Git workflow, commit conventions, and release process for this Qt application. The goal is to keep `main` stable while allowing ongoing development on `develop`.

## 1. Branching strategy

| Branch | Purpose | Rules |
| --- | --- | --- |
| `main` | Stable, tested application | Update through pull requests only. |
| `develop` | Integration branch for ongoing work | Commit and push day-to-day changes here. |
| `feature/<short-name>` *(optional)* | Isolated work on a larger feature | Branch from `develop`; merge back into `develop` through a pull request. |
| `hotfix/<short-name>` *(optional)* | Urgent fix to the stable release | Branch from `main`; merge into `main`, then bring the fix into `develop`. |

**Default workflow:** Use `main` and `develop`. Create feature branches only when their isolation is useful.

```text
main ────────●────────────────────●──────────  stable
             \                    ↑
              ●────●────●─────────┘
              develop         tested PR
```

## 2. Initial repository setup

Clone the repository (which should have `main` as its default branch):

```bash
git clone https://github.com/USERNAME/REPOSITORY.git
cd REPOSITORY
```

For a new repository, create and publish `develop` **once**:

```bash
git switch -c develop
git push -u origin develop
```

If `develop` already exists on GitHub, switch to it instead:

```bash
git fetch origin
git switch develop
```

If Git cannot find the remote branch automatically, use:

```bash
git switch --track origin/develop
```

Replace `USERNAME` and `REPOSITORY` with the actual GitHub repository path.

## 3. Daily development

Start each session on the development branch and get its latest changes:

```bash
git switch develop
git pull --ff-only origin develop
```

Make and test your changes in Qt Creator. Before committing, review what will be included:

```bash
git status
git diff
```

Stage the intended files, check the staged changes, commit, and push:

```bash
git add path/to/file.cpp path/to/file.h
git diff --cached
git commit -m "feat: add settings dialog"
git push origin develop
```

Use `git add .` only after confirming that your `.gitignore` excludes generated files and that all changes shown by `git status` belong in the commit. Prefer small commits containing one logical change.

### Optional: Work in a feature branch

For work that benefits from isolation:

```bash
git switch develop
git pull --ff-only origin develop
git switch -c feature/settings-dialog
# Make changes, then stage and commit them.
git push -u origin feature/settings-dialog
```

Open a pull request from `feature/settings-dialog` into `develop`. Once merged, switch back to `develop` and pull its latest changes. Delete the feature branch when no longer needed; **do not delete `develop`**.

## 4. Commit message convention

Use a simplified **Conventional Commits** format:

```text
<type>(optional-scope): <short, imperative description>

[optional body: explain why and important implementation details]

[optional footer: issue references or breaking-change notes]
```

The most useful commit types for this project are:

| Type | Use for | Example |
| --- | --- | --- |
| `feat` | New user-facing functionality | `feat: add application settings dialog` |
| `fix` | Correcting a bug | `fix: prevent crash when closing main window` |
| `refactor` | Restructuring code without changing behavior | `refactor: extract window initialization into helper` |
| `docs` | Documentation-only edits | `docs: document local build instructions` |
| `test` | Adding or changing tests | `test: cover invalid configuration input` |
| `chore` | Routine maintenance not covered by another type | `chore: update gitignore rules` |
| `build` | Build scripts, CMake, or dependencies | `build: configure Qt Widgets target in CMake` |
| `ci` | Continuous-integration configuration | `ci: add automated build workflow` |
| `perf` | Performance improvements | `perf: reduce unnecessary UI refreshes` |
| `style` | Formatting-only changes, not UI features | `style: apply consistent code formatting` |
| `revert` | Reverting a previous change | `revert: remove faulty settings initialization` |

**About `change:`:** You can use it informally, but it is **not a standard Conventional Commits type**. For consistency, prefer the more precise `feat:`, `fix:`, `refactor:`, `docs:`, or `chore:` instead.

### Commit-writing rules

1. **One purpose per commit.** Separate unrelated bug fixes, features, and documentation updates.
2. **Use the imperative mood.** Write `add`, `fix`, or `remove`, rather than `added`, `fixed`, or `changes`.
3. **Be specific.** Explain what changed, not merely that files were modified.
4. **Keep the subject concise.** Aim for 50 characters; keep it under roughly 72 when possible. Do not end it with a period.
5. **Use lowercase types.** For example, `feat:` instead of `Feat:`.
6. **Use a scope when helpful.** For example, `fix(settings): handle missing configuration file`.
7. **Explain *why* in the body** when the reason is not obvious. Leave a blank line after the subject.
8. **Do not commit secrets or generated build output.** Review staged changes before committing.

**Good:**

```text
feat(ui): add dark mode toggle
fix(config): use defaults when settings file is missing
refactor(main-window): separate UI setup from event handling
docs: add development workflow
build: exclude generated Qt files from source tree
```

**Avoid:**

```text
update
fixed stuff
change: changes
feat: did some work
WIP
```

For a more involved change, use a subject and body:

```text
fix(settings): preserve user preferences on restart

Save the settings before the main window closes so that the next
application launch restores the previous configuration.
```

For a breaking change, include a `BREAKING CHANGE:` footer explaining its impact and any required migration.

## 5. Testing before merging

Before moving code into `main`:

- [ ] The application builds successfully in the project's supported Qt configuration.
- [ ] The application launches and the changed behavior works as intended.
- [ ] Relevant existing features still work (basic regression testing).
- [ ] Automated tests pass, **if the project has automated tests**.
- [ ] Debug output, temporary code, credentials, and accidental generated files are not included.
- [ ] The code and documentation reflect the final behavior.

For Qt projects, keep local build output (such as `build/` and `cmake-build-*/`), compiler outputs, and Qt Creator's per-user `*.pro.user` / `CMakeLists.txt.user` files out of Git where applicable. Retain project sources such as `.cpp`, `.h`, `.ui`, `.qrc`, and `CMakeLists.txt`.

## 6. Promoting `develop` to `main`

When the development branch is tested and feature-complete:

1. Commit and push any remaining changes to `develop`.
2. On GitHub, open a **pull request** with **base: `main`** and **compare: `develop`**.
3. Describe what changed, how it was tested, and any known limitations.
4. Review the diff; resolve conflicts and ensure required checks pass.
5. Merge the pull request. Keep both long-lived branches.

A pull request helps preserve a clear review and integration history. Do not commit directly to `main` as part of normal development.

### Sync after the merge

Because the pull request is merged on GitHub, update your local branches and carry the merged result back into `develop`:

```bash
git switch main
git pull --ff-only origin main

git switch develop
git pull --ff-only origin develop
git merge main
git push origin develop
```

If Git reports merge conflicts, resolve them, review and test the result, then finish the merge and push. If your repository uses **squash merging**, this sync is particularly important for keeping the two branches aligned.

## 7. GitHub repository safeguards

Create a branch ruleset targeting `main` under **Settings → Rules → Rulesets**. Recommended settings:

- Require a pull request before merging.
- Prevent direct pushes to `main` (including by the maintainer, if desired).
- Require successful automated checks **once those checks are configured**.
- Optionally require approvals if collaborating with other developers.

Keep `main` as the repository's default branch so visitors see the stable version first.

## 8. Urgent fixes to a stable release (optional)

If a critical problem is discovered in `main` while `develop` contains unfinished work, create a `hotfix/<short-name>` branch from `main`. Apply and test only the fix, open a pull request into `main`, then merge the updated `main` into `develop` using the sync steps above.

Do not merge unfinished `develop` changes into `main` just to ship a hotfix.

---

**Quick reference:** Clone `main` → work and push on `develop` → test → open a PR from `develop` to `main` → merge → sync `main` back into `develop`.
