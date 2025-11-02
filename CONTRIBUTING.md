# Contributing to OpenFC2040

Keep contributions focused, documented, and easy to review. Follow the steps below for a smooth workflow.

## Before You Start

- Install the dependencies listed in `README.md` (`gcc-arm-none-eabi`, `cmake`, `ninja`, `python3`, `openocd`, `gdb-multiarch`).
- Run `./scripts/setup_workspace.sh` once to pull submodules and verify tooling.
- Sync the latest code: `git checkout main && git pull origin main && git submodule update --init --recursive`.
- Create a feature branch: `git checkout -b feature/short-description`.

## While You Work

- Build early and often (`./scripts/build.sh` under `firmware/openfc2040` or the relevant test firmware path).
- Keep changes scoped; avoid mixing unrelated fixes.
- Update documentation when processes, pin assignments, or workflows change—always refresh `docs/NEXT_STEPS.md` with progress and blockers.
- Note skipped tests in both the commit body and `docs/NEXT_STEPS.md`.

## Commit Rules

- Use Conventional Commits: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore` with optional scope.
- Subject line in imperative mood, ≤50 characters, no trailing period.
- Wrap body text at 72 characters; explain what changed and why, and reference issues (`Fixes #42`).
- Keep commits atomic; split large changes into reviewable pieces.

## Testing Checklist

- `./scripts/build.sh` (or equivalent) succeeds without unexpected warnings.
- Flash and validate on hardware when possible (`./scripts/flash.sh`, UART via `picocom -b 115200 /dev/ttyUSB0`).
- If hardware testing cannot be done, state it clearly in the commit and `docs/NEXT_STEPS.md`.

## Pull Requests

1. Rebase onto the latest `main`: `git fetch origin && git rebase origin/main`.
2. Push your branch and open a PR summarising the change, tests run, and documentation updates.
3. Respond to review comments promptly; prefer follow-up commits over rebases unless requested.
4. After approval, squash-and-merge (default) and delete the feature branch locally and remotely.

## Troubleshooting

**Resolve merge conflicts during rebase**
```bash
git status
# edit conflicting files
git add <file>
git rebase --continue
```

**Move an accidental commit off main**
```bash
git branch feature/move-commit
git reset --hard HEAD~1
git checkout feature/move-commit
git push origin feature/move-commit
```

## Reference Docs

- Build and flash instructions: `README.md`
- Debugging guide: `docs/DEBUG_GUIDE.md`
- Project context: `docs/PROJECT_CONTEXT.md`
- Git tips: `docs/GIT_QUICK_REFERENCE.md`
- PX4 contribution guidelines: <https://docs.px4.io/main/en/contribute/>

Questions? Open an issue or reach out to the maintainers. Steady commits and clear notes keep the project healthy.
