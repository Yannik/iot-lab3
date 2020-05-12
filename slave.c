/**
 * \file
 *         Lab3 Slave
 * \author
 *         Yannik Sembritzki <yannik@sembritzki.org>
 *
 */

#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/leds.h"
#include "command.h"
#include "node-id.h"

#include <string.h>
#include <stdio.h> /* For printf() */
#include <random.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Slave"
#define LOG_LEVEL LOG_LEVEL_INFO

#include "helpers.h"
#include "config.h"

/* Configuration */
#define SEND_INTERVAL (120 * CLOCK_SECOND)
uint8_t seq_id_received[NUM_NODES] = {[0 ... NUM_NODES-1] = 0};

/*---------------------------------------------------------------------------*/
PROCESS(slave_process, "Slave");
AUTOSTART_PROCESSES(&slave_process);

/*---------------------------------------------------------------------------*/

void sendData()
{
  NETSTACK_NETWORK.output(NULL);
}

void input_callback(const void *data, uint16_t len,
                    const linkaddr_t *src, const linkaddr_t *dest) {
  if (len != sizeof(struct command))
    return;

  static struct command cmd;
  memcpy(&cmd, data, sizeof(cmd));
  if (cmd.command == COMMAND_SEND_TEMP) {
    if (cmd.seq_id <= seq_id_received[cmd.sender_id] && seq_id_received[cmd.sender_id] - cmd.seq_id < 128)
      return;

    int node_id = linkaddr_to_node_id(src);
    //LOG_INFO("Received temp %d from %u via %u, seq_id %u\n", cmd.data, cmd.sender_id, node_id, cmd.seq_id);
    seq_id_received[cmd.sender_id] = cmd.seq_id;

    nullnet_buf = (uint8_t *)&cmd;
    nullnet_len = sizeof(cmd);
    static struct ctimer timer;
    ctimer_set(&timer, CLOCK_SECOND * (random_rand() % 1000) / 1000, &sendData, NULL);

  } else {
    log_unknown_command(cmd, src);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(slave_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer periodic_timer;
  static struct command cmd;
  cmd.sender_id = node_id;
  cmd.seq_id = 1;
  cmd.command = COMMAND_SEND_TEMP;
  cmd.hops = 1;

  nullnet_buf = (uint8_t *)&cmd;
  nullnet_len = sizeof(cmd);
  nullnet_set_input_callback(input_callback);

  fix_randomness(&linkaddr_node_addr);
  etimer_set(&periodic_timer, CLOCK_SECOND * node_id / 5);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    // Generate random temp between -100 and 100
    cmd.data = (random_rand() % 200) - 100;

    LOG_INFO("Sending Temperature %d, seq_id %u\n", cmd.data, cmd.seq_id);
    NETSTACK_NETWORK.output(NULL);

    cmd.seq_id++;
    etimer_reset(&periodic_timer);
  }  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
