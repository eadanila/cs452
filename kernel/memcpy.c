// TODO Make .h

// Copied from https://github.com/gcc-mirror/gcc/blob/master/libgcc/memcpy.c
void *
memcpy (void *dest, const void *src, unsigned int len)
{
  char *d = dest;
  const char *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}