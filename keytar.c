#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>

static void press_key(char key)
{

}
#endif

static unsigned char counter = 0;
static int previous_keys = 0;

struct key_info {
	const char *name;
	char event;
	unsigned char midi;
};

static struct key_info key_infos[] = {
{"c5",  'y', 72},
{"b4",  'x', 71},
{"a4#", 'w', 70},
{"a4",  'v', 69},
{"g4#", 'u', 68},
{"g4",  't', 67},
{"f4#", 's', 66},
{"f4",  'r', 65},
{"e4",  'q', 64},
{"d4#", 'p', 63},
{"d4",  'o', 62},
{"c4#", 'n', 61},
{"c4",  'm', 60},
{"b3",  'l', 59},
{"a3#", 'k', 58},
{"a3",  'j', 57},
{"g3#", 'i', 56},
{"g3",  'h', 55},
{"f3#", 'g', 54},
{"f3",  'f', 53},
{"e3",  'e', 52},
{"d3#", 'd', 51},
{"d3",  'c', 50},
{"c3#", 'b', 49},
{"c3",  'a', 48}
};

static void dump_buffer(const char *msg, unsigned char *buf, int len)
{
	int i;
	printf("%s: ", msg);
	for (i = 0; i < len; i++)
		printf("%02hhx ", buf[i]);
	printf("\n");
	#ifdef WIN32
		Sleep(500);
	#else
		//usleep(1000);
	#endif
}

static int count_bits(int n)
{
    n = ((0xaaaaaaaa & n) >> 1) + (0x55555555 & n);
    n = ((0xcccccccc & n) >> 2) + (0x33333333 & n);
    n = ((0xf0f0f0f0 & n) >> 4) + (0x0f0f0f0f & n);
    n = ((0xff00ff00 & n) >> 8) + (0x00ff00ff & n);
    n = ((0xffff0000 & n) >> 16) + (0x0000ffff & n);
    return n;
}

const char code_key_press[] = {0x00, 0x00, 0x08, 0x80, 0x80};
const char code_up[] =    {0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f};
const char code_right[] = {0x00, 0x00, 0x02, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f};
const char code_down[] =  {0x00, 0x00, 0x04, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f};
const char code_left[] =  {0x00, 0x00, 0x06, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f};
const char code_minus[] = {0x00, 0x01, 0x08, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f};
const char code_plus[] =  {0x00, 0x02, 0x08, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f};
const char code_1[] =     {0x01, 0x00, 0x08, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f};
const char code_2[] =     {0x08, 0x00, 0x08, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f};
const char code_A[] =     {0x02, 0x00, 0x08, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f};
const char code_B[] =     {0x04, 0x00, 0x08, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f};

static void decode(unsigned char *buf, int len)
{
	// 00 00 08 80 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 e0 00 0b

	if (len != 27) {
		dump_buffer("len?", buf, len);
		return;
	}

	if (buf[25] == counter)
		return;
	counter = buf[25];

	// Check for key press/release
	if (memcmp(buf, code_key_press, sizeof(code_key_press)) == 0) {
		int keys = (buf[5] << 17) | (buf[6] << 9) | (buf[7] << 1);

		unsigned char key_pressure[5];
		memcpy(key_pressure, &buf[8], 5);
		int keys_pressed = 0;
		for (int i = 0; i < sizeof(key_pressure); i++) {
			if (key_pressure[i] > 0)
				keys_pressed++;
		}
		if (count_bits(keys) < keys_pressed) {
			// c6 is pressed
			keys = keys | 1;
		}

		int delta_keys = keys ^ previous_keys;
		for (int i = 0; i < 25; i++) {
			int key_pressed = keys & (1 << i);
			if (delta_keys & (1 << i)) {
				printf("Key %s %s\n", key_infos[i].name, key_pressed ? "pressed" : "released");
			}
		}
		previous_keys = keys;
	} else if (memcmp(buf, code_up, sizeof(code_up)) == 0) {
		printf("Up\n");
	} else if (memcmp(buf, code_down, sizeof(code_down)) == 0) {
		printf("Down\n");
	} else if (memcmp(buf, code_right, sizeof(code_right)) == 0) {
		printf("Right\n");
	} else if (memcmp(buf, code_left, sizeof(code_left)) == 0) {
		printf("Left\n");
	} else if (memcmp(buf, code_minus, sizeof(code_minus)) == 0) {
		printf("-\n");
	} else if (memcmp(buf, code_plus, sizeof(code_plus)) == 0) {
		printf("+\n");
	} else if (memcmp(buf, code_1, sizeof(code_1)) == 0) {
		printf("1\n");
	} else if (memcmp(buf, code_2, sizeof(code_2)) == 0) {
		printf("2\n");
	} else if (memcmp(buf, code_A, sizeof(code_A)) == 0) {
		printf("A\n");
	} else if (memcmp(buf, code_B, sizeof(code_B)) == 0) {
		printf("B\n");
	} else {
		dump_buffer("unknown", buf, len);
	}
}

int main(int argc, char* argv[])
{
	if (hid_init()) {
		printf("hid_init failed?\n");
		return 1;
	}

	hid_device *handle = hid_open(0x1bad, 0x3330, NULL);
	if (!handle) {
		printf("Can't find the keytar. Make sure that it's plugged in.\n");
 		return 1;
	}

    printf("Ready to receive events from the keytar. Press CTRL+C to exit.\n");
    for (;;) {
        unsigned char buf[256];
		int res = 0;
		while (res == 0) {
			res = hid_read(handle, buf, sizeof(buf));
			if (res == 0)
				printf("waiting?\n");
			if (res < 0) {
				printf("Unable to read()\n");
				goto cleanup;
			}
		}

		decode(buf, res);
	}

cleanup:
	hid_close(handle);
	hid_exit();

	return 0;
}
