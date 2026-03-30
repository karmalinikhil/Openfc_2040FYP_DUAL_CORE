#include <nuttx/config.h>
#include <nuttx/serial/serial.h>
int test_size(void) {
    return sizeof(struct uart_dev_s);
}
