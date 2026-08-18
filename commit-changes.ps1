# Commit changes script
# Run this in PowerShell or Git Bash to commit the staged changes

# Add all modified files
git add -A

# Commit with a descriptive message
git commit -m "feat: Add status effect system and inventory integration

- Added status effect component with duration, stacks, and replication
- Added movement config data asset for animation-driven movement
- Integrated status effects with inventory system
- Updated HUD widget to display active status effects
- Updated .gitignore for Unreal Engine build artifacts"