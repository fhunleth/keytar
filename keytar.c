#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"

enum press_release {
	PR_PRESSED,
	PR_RELEASED,
	PR_BOTH
};

#ifdef __APPLE__
// OSX
#include <ApplicationServices/ApplicationServices.h>
#include <unistd.h>

static void press_key(enum key_mapping key, enum press_release pr)
{

}
#else
#ifdef _WIN32
// Windows
#include <windows.h>

enum key_mapping {
	KEYTAR_C3  = 'A',
	KEYTAR_C3S = 'B',
	KEYTAR_D3  = 'C',
	KEYTAR_D3S = 'D',
	KEYTAR_E3  = 'E',
	KEYTAR_F3  = 'F',
	KEYTAR_F3S = 'G',
	KEYTAR_G3  = 'H',
	KEYTAR_G3S = 'I',
	KEYTAR_A3  = 'J',
	KEYTAR_A3S = 'K',
	KEYTAR_B3  = 'L',
	KEYTAR_C4  = 'M',
	KEYTAR_C4S = 'N',
	KEYTAR_D4  = 'O',
	KEYTAR_D4S = 'P',
	KEYTAR_E4  = 'Q',
	KEYTAR_F4  = 'R',
	KEYTAR_F4S = 'S',
	KEYTAR_G4  = 'T',
	KEYTAR_G4S = 'U',
	KEYTAR_A4  = 'V',
	KEYTAR_A4S = 'W',
	KEYTAR_B4  = 'X',
	KEYTAR_C5  = 'Y',

    KEYTAR_UP  = VK_UP,
    KEYTAR_DOWN  = VK_DOWN,
    KEYTAR_LEFT  = VK_LEFT,
    KEYTAR_RIGHT  = VK_RIGHT,

    KEYTAR_PLUS  = VK_OEM_PLUS,
    KEYTAR_MINUS  = VK_OEM_MINUS,

    KEYTAR_1  = '1',
    KEYTAR_2  = '2',
    KEYTAR_A  = '3',
    KEYTAR_B  = '4',
};

static void press_key(enum key_mapping key, enum press_release pr)
{
	INPUT inputs[2];
	UINT count = 1;

    memset(inputs, 0, sizeof(inputs));

	inputs[0].type = INPUT_KEYBOARD;
	inputs[0].ki.wVk = key;

	switch (pr) {
		case PR_PRESSED:
			inputs[0].ki.dwFlags = 0;
			break;
		case PR_RELEASED:
			inputs[0].ki.dwFlags = KEYEVENTF_KEYUP;
			break;
		case PR_BOTH:
			inputs[0].ki.dwFlags = 0;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = inputs[0].ki.wVk;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			count = 2;
			break;
	}

	SendInput(count, inputs, sizeof(INPUT));
}

#else
// Linux
#include <unistd.h>

#endif
#endif

static unsigned char counter = 0;
static int previous_keys = 0;

// These were captured. There's some logic behind their structure, but for simplicity
// and laziness, they're shown as raw bytes below.
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

struct key_info {
	const char *name;
	unsigned char midi;
	enum key_mapping event;
};

static struct key_info key_infos[] = {
{"c5",  72, KEYTAR_C5},
{"b4",  71, KEYTAR_B4},
{"a4#", 70, KEYTAR_A4S},
{"a4",  69, KEYTAR_A4},
{"g4#", 68, KEYTAR_G4S},
{"g4",  67, KEYTAR_G4},
{"f4#", 66, KEYTAR_F4S},
{"f4",  65, KEYTAR_F4},
{"e4",  64, KEYTAR_E4},
{"d4#", 63, KEYTAR_D4S},
{"d4",  62, KEYTAR_D4},
{"c4#", 61, KEYTAR_C4S},
{"c4",  60, KEYTAR_C4},
{"b3",  59, KEYTAR_B3},
{"a3#", 58, KEYTAR_A3S},
{"a3",  57, KEYTAR_A3},
{"g3#", 56, KEYTAR_G3S},
{"g3",  55, KEYTAR_G3},
{"f3#", 54, KEYTAR_F3S},
{"f3",  53, KEYTAR_F3},
{"e3",  52, KEYTAR_E3},
{"d3#", 51, KEYTAR_D3S},
{"d3",  50, KEYTAR_D3},
{"c3#", 49, KEYTAR_C3S},
{"c3",  48, KEYTAR_C3}
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
				press_key(key_infos[i].event, key_pressed ? PR_PRESSED : PR_RELEASED);
				printf("Key %s %s\n", key_infos[i].name, key_pressed ? "pressed" : "released");
			}
		}
		previous_keys = keys;
	} else if (memcmp(buf, code_up, sizeof(code_up)) == 0) {
		press_key(KEYTAR_UP, PR_BOTH);
		printf("Up\n");
	} else if (memcmp(buf, code_down, sizeof(code_down)) == 0) {
		press_key(KEYTAR_DOWN, PR_BOTH);
		printf("Down\n");
	} else if (memcmp(buf, code_right, sizeof(code_right)) == 0) {
		press_key(KEYTAR_RIGHT, PR_BOTH);
		printf("Right\n");
	} else if (memcmp(buf, code_left, sizeof(code_left)) == 0) {
		press_key(KEYTAR_LEFT, PR_BOTH);
		printf("Left\n");
	} else if (memcmp(buf, code_minus, sizeof(code_minus)) == 0) {
		press_key(KEYTAR_MINUS, PR_BOTH);
		printf("-\n");
	} else if (memcmp(buf, code_plus, sizeof(code_plus)) == 0) {
		press_key(KEYTAR_PLUS, PR_BOTH);
		printf("+\n");
	} else if (memcmp(buf, code_1, sizeof(code_1)) == 0) {
		press_key(KEYTAR_1, PR_BOTH);
		printf("1\n");
	} else if (memcmp(buf, code_2, sizeof(code_2)) == 0) {
		press_key(KEYTAR_2, PR_BOTH);
		printf("2\n");
	} else if (memcmp(buf, code_A, sizeof(code_A)) == 0) {
		press_key(KEYTAR_A, PR_BOTH);
		printf("A\n");
	} else if (memcmp(buf, code_B, sizeof(code_B)) == 0) {
		press_key(KEYTAR_B, PR_BOTH);
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
