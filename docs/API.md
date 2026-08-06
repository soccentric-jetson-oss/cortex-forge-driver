# API Reference

## Public API
All public functions, classes, and interfaces are documented in the header files
under `include/` and `lib/include/`. The API follows these conventions:
- C drivers: Functions return 0 on success, negative errno on failure
- C++ servers: Methods return grpc::Status with descriptive error codes
- Python GUI: Methods raise typed exceptions with user-friendly messages

## Stability
The API is currently in **v0.1.0 (development)**. Breaking changes may occur.
