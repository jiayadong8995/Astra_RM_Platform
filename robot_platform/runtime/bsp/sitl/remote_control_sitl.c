#include "../drivers/remote/dbus/remote_control.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PLATFORM_SITL_REMOTE_PORT 9004

typedef struct __attribute__((packed))
{
  int16_t channels[5];
  uint8_t switches[2];
  int16_t mouse[3];
  uint8_t mouse_buttons[2];
  uint16_t keyboard_mask;
} platform_sitl_remote_packet_t;

static int platform_remote_control_ensure_socket(void);
static void platform_remote_control_poll(void);
static void platform_remote_control_reset(void);
static int16_t platform_remote_abs(int16_t value);

static int g_remote_sock = -1;
static bool g_remote_packet_seen;

RC_ctrl_t rc_ctrl;
uint8_t sbus_rx_buf[RC_FRAME_LENGTH];

const RC_ctrl_t *get_remote_control_point(void)
{
  platform_remote_control_poll();
  return &rc_ctrl;
}

uint8_t RC_data_is_error(void)
{
  platform_remote_control_poll();

  if (!g_remote_packet_seen)
  {
    platform_remote_control_reset();
    return 1U;
  }

  if (platform_remote_abs(rc_ctrl.rc.ch[0]) > 700
      || platform_remote_abs(rc_ctrl.rc.ch[1]) > 700
      || platform_remote_abs(rc_ctrl.rc.ch[2]) > 700
      || platform_remote_abs(rc_ctrl.rc.ch[3]) > 700
      || rc_ctrl.rc.s[0] == 0
      || rc_ctrl.rc.s[1] == 0)
  {
    platform_remote_control_reset();
    return 1U;
  }

  return 0U;
}

void sbus_to_rc(volatile const uint8_t *sbus_buf_local, RC_ctrl_t *rc_ctrl_local)
{
  (void)sbus_buf_local;
  (void)rc_ctrl_local;
}

static int platform_remote_control_ensure_socket(void)
{
  struct sockaddr_in addr;
  int flags = 0;

  if (g_remote_sock >= 0)
  {
    return 0;
  }

  g_remote_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (g_remote_sock < 0)
  {
    return -1;
  }

  flags = fcntl(g_remote_sock, F_GETFL, 0);
  if (flags >= 0)
  {
    (void)fcntl(g_remote_sock, F_SETFL, flags | O_NONBLOCK);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PLATFORM_SITL_REMOTE_PORT);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  if (bind(g_remote_sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
  {
    close(g_remote_sock);
    g_remote_sock = -1;
    return -1;
  }

  return 0;
}

static void platform_remote_control_poll(void)
{
  platform_sitl_remote_packet_t packet = {0};
  ssize_t received = 0;

  if (platform_remote_control_ensure_socket() != 0)
  {
    return;
  }

  while ((received = recv(g_remote_sock, &packet, sizeof(packet), 0)) == (ssize_t)sizeof(packet))
  {
    rc_ctrl.rc.ch[0] = packet.channels[0];
    rc_ctrl.rc.ch[1] = packet.channels[1];
    rc_ctrl.rc.ch[2] = packet.channels[2];
    rc_ctrl.rc.ch[3] = packet.channels[3];
    rc_ctrl.rc.ch[4] = packet.channels[4];
    rc_ctrl.rc.s[0] = (char)packet.switches[0];
    rc_ctrl.rc.s[1] = (char)packet.switches[1];
    rc_ctrl.mouse.x = packet.mouse[0];
    rc_ctrl.mouse.y = packet.mouse[1];
    rc_ctrl.mouse.z = packet.mouse[2];
    rc_ctrl.mouse.press_l = packet.mouse_buttons[0];
    rc_ctrl.mouse.press_r = packet.mouse_buttons[1];
    rc_ctrl.key.v = packet.keyboard_mask;
    g_remote_packet_seen = true;
  }
}

static void platform_remote_control_reset(void)
{
  memset(&rc_ctrl, 0, sizeof(rc_ctrl));
  rc_ctrl.rc.s[0] = RC_SW_DOWN;
  rc_ctrl.rc.s[1] = RC_SW_DOWN;
}

static int16_t platform_remote_abs(int16_t value)
{
  return (value >= 0) ? value : (int16_t)-value;
}
