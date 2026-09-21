
# TaskManager++ — Agent Instructions

## 1. Project Overview

TaskManager++ is a Windows desktop application for system
performance monitoring and diagnostics.

Its primary purpose is to consolidate Windows errors,
system events, and performance information into a
single, accessible interface.

Target platform: Windows 11.

Technology stack:
- C++: Application logic
- Qt: Desktop interface
- Windows API: Performance and event collection
- SQLite: Persistent local storage
- CMake: Build configuration
- GoogleTest: Automated testing

The project is currently under development.

Do not represent planned functionality as implemented.

---

## 2. Required Project Context

Before starting work:

1. Read README.md for project goals and current status.
2. Read docs/REQUIREMENTS.md for the Version 1 scope.
3. Read docs/ARCHITECTURE.md if it exists and the task
   affects application structure.
4. Read docs/GIT_WORKFLOW.md before Git operations.
5. Inspect relevant existing code before modifying it.

Do not assume that documentation accurately describes
the current implementation. Verify against the code
when necessary.

If documentation conflicts with implementation, report
the discrepancy and resolve it within the task scope.

---

## 3. Development Workflow

For each development task:

1. Understand the requested change and relevant code.
2. Identify affected components.
3. Plan the implementation before making substantial
   changes.
4. Implement the smallest complete solution that meets
   the requirements.
5. Add or update relevant automated tests.
6. Build and test the affected components.
7. Review the changes for unintended modifications.
8. Update documentation when required.
9. Summarize the results and any remaining issues.

Prefer incremental changes over large rewrites.

Avoid unrelated refactoring unless it is necessary
for the requested task.

Do not introduce additional dependencies without
explaining their purpose.

---

## 4. Code Quality Standards

- Use modern C++ supported by the project configuration.
- Prefer RAII and standard library facilities.
- Use clear, descriptive names.
- Follow the existing code style.
- Avoid unnecessary abstraction and overengineering.
- Separate application logic from presentation logic.
- Handle failures explicitly and provide useful errors.
- Avoid blocking the Qt GUI thread.
- Use appropriate synchronization for shared state.

Keep Windows-specific code isolated from the UI
where practical.

Do not modify generated files or build artifacts.

---

## 5. Qt Creator and MCP Usage

Use the configured Qt Creator MCP integration when
appropriate and available.

It may be used to:
- Inspect the active project.
- Build the application.
- Review compiler errors.
- Debug the application.
- Inspect relevant project information.

Do not assume the MCP server is connected.

If the integration is unavailable, report the limitation
and use the project's normal CMake workflow when possible.

Do not modify Qt Creator configuration or project kits
unnecessarily.

Never claim that a build or debugging operation succeeded
unless its result has been verified.

---

## 6. Windows Diagnostics Requirements

System event collection must be read-only in Version 1.

- Do not modify or delete Windows Event Logs.
- Do not change system configuration.
- Do not automatically terminate processes.
- Do not require administrator privileges unless a
  feature genuinely needs them.
- Handle inaccessible event sources gracefully.
- Avoid collecting unnecessary personal information.
- Do not record credentials or other secrets.

Identify events using appropriate source information,
including log, provider, and event identifiers.

Never assume that events occurring close together
necessarily have the same root cause.

Clearly distinguish recorded facts from possible
explanations.

---

## 7. Testing and Verification

All significant functionality must be tested.

Prioritize tests for:
- Event parsing and normalization.
- Event filtering and classification.
- Duplicate detection.
- Database operations.
- Incident correlation.
- Performance calculations.

Use sample event data where possible to make tests
deterministic.

After changes:
- Run relevant tests.
- Build the affected target.
- Check for compiler errors and warnings.
- Run broader tests when shared behavior changes.

Do not report tests as passing unless executed.

If tests cannot run, explain why and identify what
remains unverified.

---

## 8. Documentation Maintenance

Keep project documentation synchronized with changes.

### README.md

Update the README when changes affect:
- User-facing functionality.
- Installation or build instructions.
- Dependencies or requirements.
- Supported platforms.
- Major project milestones.

Do not update the README for every minor internal change.

Only mark roadmap items complete after functionality
has been implemented and verified.

### CHANGELOG.md

Update the Unreleased section when a task introduces
a notable change.

Use these categories when applicable:
- Added
- Changed
- Fixed
- Removed
- Security

Write concise, human-readable entries describing
observable changes.

Do not automatically generate a changelog entry for
every commit, minor refactor, or formatting change.

Do not invent release versions or release dates.

### Architecture Documentation

Update docs/ARCHITECTURE.md when changes affect:
- Major components or responsibilities.
- Module boundaries.
- Data flow.
- Storage design.
- Significant technology decisions.

### Requirements Documentation

Do not silently expand Version 1 scope.

If a requested feature is outside the defined scope,
identify it as additional work.

Update requirements when the user explicitly approves
a scope change.

---

## 9. Git Workflow

Follow docs/GIT_WORKFLOW.md when available.

Default workflow:
- main: Stable, tested code.
- develop: Integration and active development.

Use feature branches when appropriate.

Follow Conventional Commit-style messages:

feat: Add new functionality
fix: Correct existing functionality
refactor: Restructure code without changing behavior
docs: Update documentation
test: Add or modify tests
build: Update build configuration
chore: Perform maintenance

Prefer standard types such as feat, fix, refactor, and
docs over ambiguous types such as change.

Keep commits focused and descriptive.

Do not automatically:
- Commit changes.
- Push branches.
- Merge into main.
- Rewrite Git history.
- Discard uncommitted work.

Perform these actions only when explicitly requested.

---

## 10. Definition of Done

A development task is complete when:

- The requested behavior is implemented.
- Relevant tests have been added or updated.
- Verification has been performed where possible.
- No known regression remains unreported.
- Relevant documentation is updated.
- Changes remain within the agreed scope.

If any condition is not satisfied, clearly identify
the remaining work.

---

## 11. Final Response Requirements

At the end of each development task, provide:

1. Summary of changes made.
2. Files modified.
3. Tests and build results.
4. Documentation updated, if applicable.
5. Known issues or remaining work.

Distinguish completed work from incomplete or
unverified work.

Do not claim successful implementation without
supporting evidence.