bin2hex.exe --i x318.vlw --o x318-2.h
echo const uint8_t x318[] PROGMEM = { > x318.h
cat x318-2.h >> x318.h
echo }; >> x318.h
del x318-2.h