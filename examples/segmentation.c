/* examples - segmentation.c
 *
 * spacecan_lib - By astrobyte 17/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/

#include <spacecan.h>
#include <stdio.h>

static void print_frame(spacecan_frame_t *f){
  printf("ID=0x%03X DLC=%d DATA=", f->can_id, f->dlc);
  for (int i = 0; i < f->dlc; i++)
    printf("%02X ", f->buffer[i]);
  printf("\n");
}

int main(void) {
 uint8_t data[] = {
    0xDE, 0xAD, 0xBE, 0xEF,
    0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08,
    0xAA, 0xBB, 0xCC
  };

  size_t len = sizeof(data);

  spacecan_frame_t frames[SC_MAX_FRAGMENTS];
  size_t frame_count;

  sc_fragment_packet(CANID_REP, data, len, frames, &frame_count);

  printf("Generated %zu fragments:\n", frame_count);
  for (size_t i = 0; i < frame_count; i++) {
    print_frame(&frames[i]);
  }


  spacecan_reassembly_ctx_t ctx;
  sc_reassembly_init(&ctx);
  uint8_t reassembled[MAX_PACKET_SIZE];
  size_t reassembled_len;

  /* Simulate reverse order reception */
  for (int i = frame_count - 1; i >= 0; i--) {
    int r = sc_reassembly_packets(&ctx,
                                  &frames[i],
                                  0,
                                  reassembled,
                                  &reassembled_len);

    if (r == 1) {
      printf("Packet reassembled (%zu bytes):\n",
              reassembled_len);

      for (size_t j = 0; j < reassembled_len; j++)
        printf("%02X ", reassembled[j]);

      printf("\n\n");

      if (reassembled_len == len &&
        memcmp(data, reassembled, len) == 0)
        printf("SUCCESS: Payload matches original\n");
      else
        printf("ERROR: Payload mismatch\n");
    }
  }

  return 0;
}
