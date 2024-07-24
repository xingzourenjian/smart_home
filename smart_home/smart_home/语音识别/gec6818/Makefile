CC = arm-linux-gcc

CPPFLAGS += -I ./inc
CPPFLAGS += -I ./inc/libxml2
LDFLAGS  += -L ./lib

LDFLAGS += -lxml2
LDFLAGS += -lz


voicectl:voicectl.c common.c yanwu_test.c lcd.c text.c uart.c wenshidu.c camera.c takePhotoYUV.c
	$(CC) $^ -o $@ $(CPPFLAGS) $(LDFLAGS) -Wall -pthread

clean:
	rm voicectl -rf
