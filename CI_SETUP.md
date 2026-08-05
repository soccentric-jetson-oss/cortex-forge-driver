# CI/CD Setup

This repository is ready for CI/CD. To enable:

1. Go to GitHub.com → Repository → Settings → Actions
2. Add the following secrets:
   - `GH_TOKEN`: A GitHub personal access token with `workflow` scope
3. Uncomment and push `.github/workflows/ci.yml`

## Local Testing
```bash
make test    # Run tests locally
make lint    # Run linters
```
