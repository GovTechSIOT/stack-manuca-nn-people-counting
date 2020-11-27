import os

os.system("mbed test -t GCC_ARM -m NUCLEO_F767ZI --profile ./tools/profiles/tiny_debug.json -n src-*,threads-*")