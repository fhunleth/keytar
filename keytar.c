#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"

// Headers needed for sleeping.
#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif

static unsigned char counter = 0;
static int previous_keys = 0;

struct key_info {
	const char *name;
	char event;
	unsigned char midi;
};

static struct key_info key_infos[] = {
{"c6",  'y', 84},
{"b5",  'x', 83},
{"a5#", 'w', 82},
{"a5",  'v', 81},
{"g5#", 'u', 80},
{"g5",  't', 79},
{"f5#", 's', 78},
{"f5",  'r', 77},
{"e5",  'q', 76},
{"d5#", 'p', 75},
{"d5",  'o', 74},
{"c5#", 'n', 73},
{"c5",  'm', 72},
{"b4",  'l', 71},
{"a4#", 'k', 70},
{"a4",  'j', 69},
{"g4#", 'i', 68},
{"g4",  'h', 67},
{"f4#", 'g', 66},
{"f4",  'f', 65},
{"e4",  'e', 64},
{"d4#", 'd', 63},
{"d4",  'c', 62},
{"c4#", 'b', 61},
{"c4",  'a', 60}
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
	if (buf[0] == 0x00 && buf[1] == 0x00 && buf[2] == 0x08 && buf[3] == 0x80 && buf[4] == 0x80) {
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
	} else {
		dump_buffer("unknown", buf, len);
	}
}

int main(int argc, char* argv[])
{
	int res;
	unsigned char buf[256];
	#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	hid_device *handle;
	int i;

#ifdef WIN32
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
#endif


	if (hid_init())
		return -1;

#if 0
	struct hid_device_info *devs, *cur_dev;
	devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;
	while (cur_dev) {
		printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		printf("\n");
		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		printf("  Product:      %ls\n", cur_dev->product_string);
		printf("  Release:      %hx\n", cur_dev->release_number);
		printf("  Interface:    %d\n",  cur_dev->interface_number);
		printf("\n");
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);
#endif

	// Set up the command buffer.
	memset(buf,0x00,sizeof(buf));
	buf[0] = 0x01;
	buf[1] = 0x81;


	// Open the device using the VID, PID,
	// and optionally the Serial number.
	////handle = hid_open(0x4d8, 0x3f, L"12345");
	handle = hid_open(0x1bad, 0x3330, NULL);
	if (!handle) {
		printf("Can't find the keytar\n");
 		return 1;
	}

#if 0
	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read manufacturer string\n");
	printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	wstr[0] = 0x0000;
	res = hid_get_product_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read product string\n");
	printf("Product String: %ls\n", wstr);
	// Read the Serial Number String
	wstr[0] = 0x0000;
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read serial number string\n");
	printf("Serial Number String: (%d) %ls", wstr[0], wstr);
	printf("\n");

	// Read Indexed String 1
	wstr[0] = 0x0000;
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read indexed string 1\n");
	printf("Indexed String 1: %ls\n", wstr);
#endif
	// Set the hid_read() function to be non-blocking.
	//hid_set_nonblocking(handle, 1);

	// Try to read from the device. There shoud be no
	// data here, but execution should not block.
	//res = hid_read(handle, buf, 17);
#if 0
	// Send a Feature Report to the device
	buf[0] = 0x2;
	buf[1] = 0xa0;
	buf[2] = 0x0a;
	buf[3] = 0x00;
	buf[4] = 0x00;
	res = hid_send_feature_report(handle, buf, 17);
	if (res < 0) {
		printf("Unable to send a feature report.\n");
	}

	memset(buf,0,sizeof(buf));

	// Read a Feature Report from the device
	buf[0] = 0x2;
	res = hid_get_feature_report(handle, buf, sizeof(buf));
	if (res < 0) {
		printf("Unable to get a feature report.\n");
		printf("%ls", hid_error(handle));
	}
	else {
		// Print out the returned buffer.
		printf("Feature Report\n   ");
		for (i = 0; i < res; i++)
			printf("%02hhx ", buf[i]);
		printf("\n");
	}
#endif
	memset(buf,0,sizeof(buf));
#if 0
	// Toggle LED (cmd 0x80). The first byte is the report number (0x1).
	buf[0] = 0x1;
	buf[1] = 0x80;
	res = hid_write(handle, buf, 17);
	if (res < 0) {
		printf("Unable to write()\n");
		printf("Error: %ls\n", hid_error(handle));
	}


	// Request state (cmd 0x81). The first byte is the report number (0x1).
	buf[0] = 0x1;
	buf[1] = 0x81;
	hid_write(handle, buf, 17);
	if (res < 0)
		printf("Unable to write() (2)\n");
#endif

        for (;;) {
	// Read requested state. hid_read() has been set to be
	// non-blocking by the call to hid_set_nonblocking() above.
	// This loop demonstrates the non-blocking nature of hid_read().
	res = 0;
	while (res == 0) {
		res = hid_read(handle, buf, sizeof(buf));
		if (res == 0)
			printf("waiting...\n");
		if (res < 0) {
			printf("Unable to read()\n");
			goto cleanup;
		}
	}

	decode(buf, res);
	}

cleanup:
	hid_close(handle);

	/* Free static HIDAPI objects. */
	hid_exit();

#ifdef WIN32
	system("pause");
#endif

	return 0;
}
