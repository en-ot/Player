bin2hex.exe --i x324.vlw --o x324-2.h
echo const uint8_t x324[] PROGMEM = { > x324.h
type x324-2.h >> x324.h
echo }; >> x324.h
rem copy /b x324-1.h+x324-2.h+x324-3.h x318.h /Y
