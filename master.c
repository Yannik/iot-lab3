/**
 * \file
 *         Lab3 Master
 * \author
 *         Yannik Sembritzki <yannik@sembritzki.org>
 *
 */

#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/leds.h"

#include <string.h>
#include <stdio.h> /* For printf() */
#include <random.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Master"
#define LOG_LEVEL LOG_LEVEL_INFO

#include "helpers.h"
#include "config.h"

/* Configuration */
#define PRINT_INTERVAL (10 * CLOCK_SECOND)

#define TEMP_NOT_RECEIVED -128
int8_t temperature[NUM_NODES] = {[0 ... NUM_NODES-1] = TEMP_NOT_RECEIVED};

/*---------------------------------------------------------------------------*/
PROCESS(master_process, "Master");
AUTOSTART_PROCESSES(&master_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
  if(len != sizeof(struct command))
    return;

  struct command cmd;
  memcpy(&cmd, data, sizeof(cmd));

  int node_id = linkaddr_to_node_id(src);
  if (node_id > NUM_NODES || cmd.sender_id > NUM_NODES) {
    LOG_INFO("An unknown sender appeared!\n");
    return;
  }

  if (cmd.command == COMMAND_SEND_TEMP) {
    LOG_INFO("Received Temp %d from %u\n", cmd.data, cmd.sender_id);
    temperature[cmd.sender_id-1] = cmd.data;
  } else {
    log_unknown_command(cmd, src);
  }
}

PROCESS_THREAD(master_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer periodic_timer;

  nullnet_set_input_callback(input_callback);
  etimer_set(&periodic_timer, PRINT_INTERVAL + 8 * CLOCK_SECOND);
  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    int max_temp = 0;
    int i;
    for (i = 0; i < NUM_NODES; i++) {
      if (temperature[i] > max_temp) {
        max_temp = temperature[i];
      }
      temperature[i] = TEMP_NOT_RECEIVED;
    }
    LOG_INFO("Max Temp received in the last 120s: %d\n", max_temp);

    etimer_set(&periodic_timer, PRINT_INTERVAL);
    etimer_reset(&periodic_timer);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
