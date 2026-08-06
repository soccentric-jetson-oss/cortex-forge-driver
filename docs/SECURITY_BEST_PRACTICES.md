# Security Best Practices

1. **Bind to localhost** by default — never expose to public networks without auth
2. **Validate all inputs** — check bounds, types, and formats before processing
3. **Use devres** — automatic resource cleanup prevents leaks
4. **Rate limit** — protect against DoS attacks on gRPC endpoints
5. **Audit dependencies** — keep all dependencies updated
6. **No hardcoded secrets** — use environment variables or secret management
