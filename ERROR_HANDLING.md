# Error Handling

## Principles
1. All functions return error codes (C/C++) or raise exceptions (Python)
2. Error messages are descriptive and actionable
3. Resources are cleaned up on failure (RAII/devres)
4. Edge cases (null, empty, overflow, timeout) are handled

## Patterns
- C drivers: return negative errno, use dev_err_probe()
- C++ servers: return grpc::Status with error codes
- Python GUI: catch exceptions, show user-friendly messages
