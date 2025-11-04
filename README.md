# LibWinHttp

LibWinHttp is a **simplified WinHTTP wrapper** designed as a Crystal Palace shared library for implant development. Its primary purpose is to facilitate the development of PICO modules that require HTTP/HTTPS transport layer communication.

The library provides a lightweight abstraction over Microsoft's WinHTTP API, hiding the complexity of session and connection management to enable straightforward HTTP/HTTPS operations with a simplified, easy-to-use interface.

## Key Features

- **Simplified API**: Easy-to-use functions for common HTTP operations (GET, POST, PUT, DELETE, HEAD, PATCH).
- **Flexible Headers & Body**: Support for custom headers and request bodies in a structured format.
- **Configurable Client**: Customizable timeout and User-Agent settings for each HTTP client instance.
- **Connection Reuse**: Efficient connection reuse for multiple requests to the same host.

## API Reference (`HTTP.h`)

### Core Types

#### `HttpHandle`
HTTP client handle containing configuration and state.
- `response_timeout_ms`: Response timeout in milliseconds (default: 30000ms).
- `user_agent`: Custom User-Agent string (default: "LibHttp/1.0").

#### `HttpMethod`
HTTP method enumeration: `GET`, `POST`, `PUT`, `DELETE`, `HEAD`, `PATCH`.

#### `HttpResponse`
Response data structure containing:
- `status_code`: HTTP status code (e.g., 200, 404, 500).
- `body`: Response body as a null-terminated string.
- `body_size`: Size of the response body in bytes.
- `content_type`: Content-Type header from the response.

#### `HttpHeader`
Single header key-value pair:
- `name`: Header name (e.g., "Content-Type").
- `value`: Header value (e.g., "application/json").

#### `HttpHeaders`
Collection of headers:
- `headers`: Array of `HttpHeader` structures.
- `count`: Number of headers in the array.

#### `HttpBody`
Request body data:
- `data`: Pointer to body bytes.
- `size`: Size of body in bytes.

#### `HttpURI`
Request URI decomposed into components:
- `host`: Hostname (e.g., "example.com").
- `port`: Port number (e.g., 80 for HTTP, 443 for HTTPS).
- `path`: Request path (e.g., "/api/endpoint").

### Core Functions

#### `HttpInit`
Creates and initializes a new HTTP client handle.
- **Parameters**:
  - `https_enabled`: 1 to enable HTTPS support, 0 to disable.
- **Returns**: Pointer to a new `HttpHandle` structure, or NULL on failure.
- **Ownership**: User owns the returned handle and must free it with `HttpDestroy()`.

#### `HttpDestroy`
Destroys and cleans up an HTTP client handle.
- **Parameters**:
  - `handle`: HTTP client handle from `HttpInit()`.

### HTTP Requests

#### `HttpRequest`
Performs an HTTP request.
- **Parameters**:
  - `handle`: HTTP client handle from `HttpInit`.
  - `method`: HTTP method to use (GET, POST, PUT, DELETE, HEAD, PATCH).
  - `uri`: URI containing host, port, and path.
  - `headers`: Headers collection (can be NULL for no headers).
  - `body`: Request body (can be NULL for GET/HEAD/DELETE).
  - `response`: Pointer to `HttpResponse` structure (will be populated).
- **Returns**: TRUE on success, FALSE on failure.

## Design Notes

- **Connection Reuse**: The same HTTP client can be reused for multiple requests to the same host. The connection is kept open and reused automatically.
- **SSL/TLS Configuration**: When `https_enabled` is set to 1, SSL certificate verification is disabled (trust all certificates) for simplicity.
- **Memory Management**: Response bodies and content-type strings are dynamically allocated and must be freed by the caller.