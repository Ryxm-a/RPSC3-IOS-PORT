// iOS stub for hidapi: no HID devices are ever exposed.
#include <hidapi.h>
#include <hidapi_darwin.h>
#include <stddef.h>

int  HID_API_EXPORT hid_init(void) { return 0; }
int  HID_API_EXPORT hid_exit(void) { return 0; }
struct hid_device_info* HID_API_EXPORT hid_enumerate(unsigned short v, unsigned short p) { (void)v; (void)p; return NULL; }
void HID_API_EXPORT hid_free_enumeration(struct hid_device_info* d) { (void)d; }
hid_device* HID_API_EXPORT hid_open_path(const char* path) { (void)path; return NULL; }
void HID_API_EXPORT hid_close(hid_device* d) { (void)d; }
int  HID_API_EXPORT hid_write(hid_device* d, const unsigned char* b, size_t n) { (void)d; (void)b; (void)n; return -1; }
int  HID_API_EXPORT hid_read(hid_device* d, unsigned char* b, size_t n) { (void)d; (void)b; (void)n; return -1; }
int  HID_API_EXPORT hid_read_timeout(hid_device* d, unsigned char* b, size_t n, int t) { (void)d; (void)b; (void)n; (void)t; return -1; }
int  HID_API_EXPORT hid_set_nonblocking(hid_device* d, int nb) { (void)d; (void)nb; return -1; }
int  HID_API_EXPORT hid_send_feature_report(hid_device* d, const unsigned char* b, size_t n) { (void)d; (void)b; (void)n; return -1; }
int  HID_API_EXPORT hid_get_feature_report(hid_device* d, unsigned char* b, size_t n) { (void)d; (void)b; (void)n; return -1; }
struct hid_device_info* HID_API_EXPORT hid_get_device_info(hid_device* d) { (void)d; return NULL; }
const wchar_t* HID_API_EXPORT hid_error(hid_device* d) { (void)d; return L"HID is not supported on iOS"; }
void HID_API_EXPORT hid_darwin_set_open_exclusive(int e) { (void)e; }
