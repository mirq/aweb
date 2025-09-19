# HTTP Compression Support

## Compression Methods

### gzip
- RFC 1952 standard
- Header: `Content-Encoding: gzip`
- Accept: `Accept-Encoding: gzip`
- Magic bytes: `1f 8b`

### deflate
- RFC 1951 (raw deflate) or RFC 1950 (zlib wrapper)
- Header: `Content-Encoding: deflate`
- Accept: `Accept-Encoding: deflate`
- Usually zlib-wrapped deflate

### zlib
- RFC 1950 standard
- Internal compression format
- Magic bytes: `78 9c` (default), `78 01`, `78 5e`, `78 da`

## Processing Headers

### Request Headers
```
Accept-Encoding: gzip, deflate
Accept-Encoding: gzip;q=1.0, deflate;q=0.8
```

### Response Headers
```
Content-Encoding: gzip
Content-Encoding: deflate
Content-Length: <compressed-size>
```

### Transfer Encoding
```
Transfer-Encoding: chunked
Transfer-Encoding: gzip, chunked
```

## Implementation Notes

- Check `Content-Encoding` header to determine decompression method
- Send `Accept-Encoding` in requests to indicate support
- Handle chunked transfer encoding separately from content encoding
- Verify magic bytes for format validation