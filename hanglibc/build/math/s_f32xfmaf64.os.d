$(common-objpfx)math/s_f32xfmaf64.os: \
 ../sysdeps/ieee754/dbl-64/s_f32xfmaf64.c ../include/stdc-predef.h \
 $(common-objpfx)libc-modules.h ../include/libc-symbols.h \
 $(common-objpfx)config.h ../sysdeps/generic/libc-symver.h \
 ../sysdeps/generic/symbol-hacks.h
../include/stdc-predef.h:
$(common-objpfx)libc-modules.h:
../include/libc-symbols.h:
$(common-objpfx)config.h:
../sysdeps/generic/libc-symver.h:
../sysdeps/generic/symbol-hacks.h: