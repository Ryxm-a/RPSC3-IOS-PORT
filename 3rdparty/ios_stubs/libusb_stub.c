// iOS stub for libusb: no USB backend is available to sandboxed apps.
#include <libusb.h>
#include <stdlib.h>

#define NOSUP LIBUSB_ERROR_NOT_SUPPORTED

int libusb_init(libusb_context** c) { if (c) *c = NULL; return 0; }
int libusb_init_context(libusb_context** c, const struct libusb_init_option o[], int n) { (void)o; (void)n; if (c) *c = NULL; return 0; }
void libusb_exit(libusb_context* c) { (void)c; }
int libusb_has_capability(uint32_t cap) { (void)cap; return 0; }
ssize_t libusb_get_device_list(libusb_context* c, libusb_device*** l) { (void)c; if (l) { *l = (libusb_device**)calloc(1, sizeof(void*)); } return 0; }
void libusb_free_device_list(libusb_device** l, int u) { (void)u; free(l); }
libusb_device* libusb_ref_device(libusb_device* d) { return d; }
void libusb_unref_device(libusb_device* d) { (void)d; }
int libusb_get_device_descriptor(libusb_device* d, struct libusb_device_descriptor* s) { (void)d; (void)s; return NOSUP; }
uint8_t libusb_get_device_address(libusb_device* d) { (void)d; return 0; }
uint8_t libusb_get_port_number(libusb_device* d) { (void)d; return 0; }
int libusb_open(libusb_device* d, libusb_device_handle** h) { (void)d; if (h) *h = NULL; return NOSUP; }
void libusb_close(libusb_device_handle* h) { (void)h; }
int libusb_get_configuration(libusb_device_handle* h, int* c) { (void)h; (void)c; return NOSUP; }
int libusb_set_configuration(libusb_device_handle* h, int c) { (void)h; (void)c; return NOSUP; }
int libusb_claim_interface(libusb_device_handle* h, int i) { (void)h; (void)i; return NOSUP; }
int libusb_release_interface(libusb_device_handle* h, int i) { (void)h; (void)i; return NOSUP; }
int libusb_set_auto_detach_kernel_driver(libusb_device_handle* h, int e) { (void)h; (void)e; return NOSUP; }
int libusb_control_transfer(libusb_device_handle* h, uint8_t t, uint8_t r, uint16_t v, uint16_t i, unsigned char* d, uint16_t l, unsigned int to) { (void)h; (void)t; (void)r; (void)v; (void)i; (void)d; (void)l; (void)to; return NOSUP; }
struct libusb_transfer* libusb_alloc_transfer(int n) { (void)n; return (struct libusb_transfer*)calloc(1, sizeof(struct libusb_transfer)); }
void libusb_free_transfer(struct libusb_transfer* t) { free(t); }
int libusb_submit_transfer(struct libusb_transfer* t) { (void)t; return NOSUP; }
int libusb_handle_events_timeout_completed(libusb_context* c, struct timeval* tv, int* done) { (void)c; (void)tv; (void)done; return 0; }
int libusb_hotplug_register_callback(libusb_context* c, int ev, int fl, int v, int p, int cl, libusb_hotplug_callback_fn cb, void* u, libusb_hotplug_callback_handle* h) { (void)c; (void)ev; (void)fl; (void)v; (void)p; (void)cl; (void)cb; (void)u; (void)h; return NOSUP; }
void libusb_hotplug_deregister_callback(libusb_context* c, libusb_hotplug_callback_handle h) { (void)c; (void)h; }
const char* libusb_error_name(int e) { (void)e; return "LIBUSB_ERROR_NOT_SUPPORTED"; }
