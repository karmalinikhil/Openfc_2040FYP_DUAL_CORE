# Contributing to OpenFC2040

Keep contributions focused, documented, and easy to review. Follow the steps below for a smooth workflow.

## Before You Start

- Install the dependencies listed in `README.md` (`gcc-arm-none-eabi`, `cmake`, `ninja-build`, `python3`, `gdb-multiarch`).
- Clone with submodules: `git clone --recursive https://github.com/Rispats/OpenFC2040_FYP.git`
- Sync the latest code: `git checkout main && git pull origin main && git submodule update --init --recursive`
- Create a feature branch: `git checkout -b feature/short-description`

## While You Work

- Build early and often: `cd PX4-Autopilot && make raspberrypi_pico_default`
- Keep changes scoped; avoid mixing unrelated fixes.
- Update documentation when processes, pin assignments, or workflows change.
- Note any blockers or skipped tests in commit messages.

## Commit Rules

- Use Conventional Commits: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore` with optional scope.
- Subject line in imperative mood, ≤50 characters, no trailing period.
- Wrap body text at 72 characters; explain what changed and why, and reference issues (`Fixes #42`).
- Keep commits atomic; split large changes into reviewable pieces.

## Testing Checklist

- Build succeeds without unexpected warnings.
- Flash and validate on hardware when possible:
  ```bash
  # Convert and flash
  cp build/raspberrypi_pico_default/raspberrypi_pico_default.uf2 /media/$USER/RPI-RP2/
  
  # Test via UART console
  picocom -b 115200 /dev/ttyUSB0
  ```
- If hardware testing cannot be done, state it clearly in the commit.

## Pull Requests

1. Rebase onto the latest `main`: `git fetch origin && git rebase origin/main`
2. Push your branch and open a PR summarising the change, tests run, and documentation updates.
3. Respond to review comments promptly; prefer follow-up commits over rebases unless requested.
4. After approval, squash-and-merge (default) and delete the feature branch.

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

Questions? Open an issue or reach out to the maintainers.
