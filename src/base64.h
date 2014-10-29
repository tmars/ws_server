#ifndef BASE64_H
#define BASE64_H

char *base64_encode(const unsigned char *,
	size_t, size_t *);

unsigned char *base64_decode(const char *,
	size_t, size_t *);

#endif /* BASE64_H */