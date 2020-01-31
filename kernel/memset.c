void *memset(void *str, int c, unsigned int n) {
    volatile unsigned char *p = str;
    for(unsigned int i = 0; i < n; i++) {
        p[i] = c;
    }
    return str;
}

