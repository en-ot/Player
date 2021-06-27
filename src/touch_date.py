import os
filename = ".pio/build/Player/src/firmware.cpp.o"
try:
    os.remove(filename)
    print("Removed", filename)
except:
    pass
