tools\esptool.exe --port COM3 --baud 2000000 write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0x610000 data/x324.prt