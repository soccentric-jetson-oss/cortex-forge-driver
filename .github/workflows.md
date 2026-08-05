# CI/CD Workflows

To enable CI/CD:
1. Create `.github/workflows/ci.yml` (see CI_SETUP.md)
2. Add a GitHub token with `workflow` scope as `GH_TOKEN` secret
3. Push the workflow file to trigger automated builds

## Planned Workflows
- Build and test on push/PR
- Static analysis (lint, type checking)
- Package build (deb, rpm, PyInstaller)
- Release automation
