# Authentication & Authorization

This project currently uses no authentication (development phase).

## Planned
- mTLS for gRPC connections (v0.2.0)
- API key authentication (v0.3.0)

## Security Note
Default configuration binds to localhost only. For production:
1. Enable mTLS between server and clients
2. Use a reverse proxy for authentication
3. Restrict network access via firewall
