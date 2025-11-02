# Quick Git Reference for OpenFC2040

## 🚀 Initial Setup (DO ONCE)

```bash
cd path/to/your/local/clone   # repo root

# Configure your identity
git config user.name "Rishi Patil"
git config user.email "your.email@example.com"

# Add all files
git add .

# Initial commit
git commit -m "feat: reorganize project structure with comprehensive documentation

- Reorganize into firmware/, hardware/, tools/, docs/ structure
- Add 6 essential documentation files
- Integrate official hardware repository (vxj9800/openFC2040)
- Verify all pin connections match PCB schematic
- Document critical battery voltage divider limitation (2S LiPo only)
- Remove unnecessary documentation files

All connections verified against openFC2040.json schematic - 100% accurate."

# Push to GitHub
git push -u origin main
```

---

## 📝 Daily Workflow (Every Time You Make Changes)

### 1. Check what changed
```bash
git status              # See modified files
git diff                # See exact changes
```

### 2. Add your changes
```bash
# Add specific files:
git add path/to/file1.c
git add path/to/file2.h

# OR add everything:
git add .
```

### 3. Commit with message
```bash
git commit -m "type(scope): brief description

- Detailed change 1
- Detailed change 2"
```

**Commit Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `refactor`: Code restructuring
- `test`: Adding tests
- `chore`: Build/tools/dependencies

**Examples:**
```bash
git commit -m "feat(imu): add LSM6DS3 driver"
git commit -m "fix(battery): correct voltage divider for 3S LiPo"
git commit -m "docs: update PIN_CONNECTIONS with new resistor values"
```

### 4. Push to GitHub
```bash
git push origin main
```

---

## 🔄 Complete Example Workflow

```bash
# 1. Made changes to board_config.h
vim firmware/openfc2040/rsp_2040/board/board_config.h

# 2. Check what changed
git status
git diff

# 3. Add the file
git add firmware/openfc2040/rsp_2040/board/board_config.h

# 4. Also update NEXT_STEPS.md (ALWAYS!)
vim docs/NEXT_STEPS.md
git add docs/NEXT_STEPS.md

# 5. Commit
git commit -m "fix(board): update battery voltage divider to 5.09:1

- Changed BOARD_BATTERY_V_DIV from 1.74 to 5.09
- Now supports 3S LiPo safely (max 12.6V)
- Verified calculation: R6=2kΩ, R7=2.7kΩ
- Updated docs/NEXT_STEPS.md"

# 6. Push
git push origin master
```

---

## ⚠️ CRITICAL: Always Update NEXT_STEPS.md

After ANY change, update `docs/NEXT_STEPS.md`:

```bash
vim docs/NEXT_STEPS.md

# Update these sections:
# - "What Was Done Last" (add today's date)
# - Move completed tasks to "Completed Tasks"
# - Add any new issues/blockers

git add docs/NEXT_STEPS.md
git commit -m "docs: update NEXT_STEPS.md after battery divider fix"
git push origin master
```

---

## 🌿 Using Branches (For Big Features)

```bash
# Create feature branch
git checkout -b feature/add-imu-driver

# Make changes, commit
git add drivers/imu/lsm6ds3/*
git commit -m "feat(imu): implement LSM6DS3 driver"

# Push branch
git push origin feature/add-imu-driver

# After testing, merge to master
git checkout master
git merge feature/add-imu-driver
git push origin master

# Delete feature branch
git branch -d feature/add-imu-driver
git push origin --delete feature/add-imu-driver
```

---

## 🆘 Common Problems

### Problem: Permission denied (publickey)
```bash
# Generate SSH key
ssh-keygen -t ed25519 -C "your.email@example.com"

# Copy public key
cat ~/.ssh/id_ed25519.pub

# Add to GitHub: Settings → SSH and GPG keys → New SSH key
```

### Problem: "Updates were rejected"
```bash
# Pull changes from GitHub first
git pull origin main --rebase

# Then push
git push origin main
```

### Problem: Accidentally committed wrong file
```bash
# Before push - undo last commit (keep changes)
git reset HEAD~1

# After push - revert commit (creates new commit)
git revert HEAD
git push origin master
```

### Problem: Want to undo changes to a file
```bash
# Discard uncommitted changes
git restore filename.c

# Unstage file (keep changes)
git reset HEAD filename.c
```

---

## 📊 Useful Commands

```bash
# View commit history
git log --oneline -10          # Last 10 commits
git log --graph --oneline      # Visual graph

# See changes in last commit
git show

# Compare with GitHub
git diff origin/master

# Pull latest from GitHub
git pull origin master
```

---

## ✅ Checklist Before Every Push

- [ ] `git status` - Check what's changing
- [ ] `git diff` - Review actual changes
- [ ] Updated `docs/NEXT_STEPS.md`
- [ ] Commit message is descriptive
- [ ] Code builds successfully (`make raspberrypi_pico_minimal`)
- [ ] No sensitive data (passwords, keys) in commit

---

## 📞 Quick Help

**See what you're about to commit:**
```bash
git diff --staged
```

**Fix last commit message:**
```bash
git commit --amend
```

**Undo git add:**
```bash
git reset
```

**See all branches:**
```bash
git branch -a
```

---

**Need more help?** See `CONTRIBUTING.md` for full guidelines.
