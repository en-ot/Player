bin2hex.exe --i x324.vlw --o x324-2.h
echo const uint8_t x324[] PROGMEM = { > x324.h
cat x324-2.h >> x324.h
echo }; >> x324.h
del x324-2.h