$(common-objpfx)string/rtld-rawmemchr-avx2-rtm.os: \
 ../sysdeps/x86_64/multiarch/rawmemchr-avx2-rtm.S \
 ../include/stdc-predef.h $(common-objpfx)libc-modules.h \
 ../include/libc-symbols.h $(common-objpfx)config.h \
 ../sysdeps/generic/libc-symver.h ../sysdeps/generic/symbol-hacks.h \
 /usr/lib/gcc/x86_64-linux-gnu/11/include/cet.h \
 ../sysdeps/x86_64/multiarch/memchr-avx2-rtm.S \
 ../sysdeps/x86_64/multiarch/memchr-avx2.S
../include/stdc-predef.h:
$(common-objpfx)libc-modules.h:
../include/libc-symbols.h:
$(common-objpfx)config.h:
../sysdeps/generic/libc-symver.h:
../sysdeps/generic/symbol-hacks.h:
/usr/lib/gcc/x86_64-linux-gnu/11/include/cet.h:
../sysdeps/x86_64/multiarch/memchr-avx2-rtm.S:
../sysdeps/x86_64/multiarch/memchr-avx2.S: