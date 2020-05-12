#ifndef COMMAND_H
#define COMMAND_H
struct command {
    uint8_t seq_id;
    uint8_t sender_id;
    uint8_t hops;
    uint8_t command;
    uint8_t data;
};

#define COMMAND_SEND_TEMP 	1

#endif
