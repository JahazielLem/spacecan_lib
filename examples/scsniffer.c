/* examples - scsniffer.c
 *
 * spacecan_lib - By astrobyte 17/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/

#include <spacecan.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <time.h>

typedef struct {
  uint32_t magic;
  uint16_t version_major;
  uint16_t version_minor;
  int32_t thiszone;
  uint32_t sigfigs;
  uint32_t snaplen;
  uint32_t network;
} pcap_global_header_t;

typedef struct {
  uint32_t ts_sec;
  uint32_t ts_usec;
  uint32_t incl_len;
  uint32_t orig_len;
} pcap_packet_header_t;

typedef struct {
  int      filter_enabled;
  uint32_t filter_can_id;

  int      pcap_write_enabled;
  const char *pcap_filename;

  int      replay_write_enable;
  const char *replay_filename;
} sniffer_config_t;

typedef struct {
  uint64_t timestamp_ns;
  spacecan_frame_t frame;
} replay_record_t;


static FILE *pcap_file = NULL;
static FILE *replay_file = NULL;

int parse_args(int argc, char *argv[], sniffer_config_t *cfg) {
  int opt;

  // Inicializar defaults
  cfg->filter_enabled = 0;
  cfg->pcap_write_enabled  = 0;
  cfg->pcap_filename  = NULL;
  cfg->replay_write_enable = 0;
  cfg->replay_filename = NULL;

  while ((opt = getopt(argc, argv, "f:o:r:h")) != -1) {
    switch (opt) {
      case 'f':
        cfg->filter_enabled = 1;
        cfg->filter_can_id = strtoul(optarg, NULL, 16);
        break;
      case 'o':
        cfg->pcap_write_enabled = 1;
        cfg->pcap_filename = optarg;
        break;
      case 'r':
        cfg->replay_write_enable = 1;
        cfg->replay_filename = optarg;
        break;
      case 'h':
      default:
        printf("Usage:\n");
        printf("  sniffer                 -> capture all\n");
        printf("  sniffer -f 0x301        -> filter CAN ID\n");
        printf("  sniffer -o file.pcap    -> write pcap\n");
        printf("  sniffer -r replay       -> write replay format\n");
        printf("  sniffer -f 0x301 -w out.pcap\n");
        return -1;
    }
  }
  return 0;
}


static void print_frame(spacecan_frame_t *f){
  printf("ID=0x%03X DLC=%d DATA=", f->can_id, f->dlc);
  for (int i = 0; i < f->dlc; i++)
    printf("%02X ", f->buffer[i]);
  printf("\n");
}

static void pcap_open(const char *filename) {
  pcap_file = fopen(filename, "wb");

  pcap_global_header_t gh = {
    .magic = 0xa1b2c3d4,
    .version_major = 2,
    .version_minor = 4,
    .thiszone = 0,
    .sigfigs = 0,
    .snaplen = 65535,
    .network = 147
  };

  fwrite(&gh, sizeof(gh), 1, pcap_file);
  fflush(pcap_file);
}

static void pcap_close(void){
  fflush(pcap_file);
  fclose(pcap_file);
}

static void replay_open(const char *filename){
  replay_file = fopen(filename, "w");
}

static void replay_close(void){
  fclose(replay_file);
}

static void replay_write(spacecan_frame_t *frame){
  replay_record_t rec;

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  rec.timestamp_ns = ts.tv_sec * 1000000000ULL + ts.tv_nsec;
  rec.frame = *frame;

  fwrite(&rec, sizeof(rec), 1, replay_file);
  fflush(replay_file);
}

static void pcap_write(spacecan_frame_t *frame, uint64_t timestamp_ns) {
  if (!pcap_file) { return; }

  pcap_packet_header_t ph;

  ph.ts_sec  = timestamp_ns / 1000000000ULL;
  ph.ts_usec = (timestamp_ns % 1000000000ULL) / 1000;
  ph.incl_len = sizeof(spacecan_frame_t);
  ph.orig_len = sizeof(spacecan_frame_t);

  fwrite(&ph, sizeof(ph), 1, pcap_file);
  fwrite(frame, sizeof(spacecan_frame_t), 1, pcap_file);
  fflush(pcap_file);
}

int main(int argc, char *argv[]) {
  sniffer_config_t cfg;

  if (parse_args(argc, argv, &cfg) < 0) {
    return 1;
  }

  if (cfg.filter_enabled) {
    printf("[Sniffer] Filter enabled: 0x%03X\n", cfg.filter_can_id);
  }

  if (cfg.pcap_write_enabled) {
    printf("[Sniffer] Writing to: %s\n", cfg.pcap_filename);
    pcap_open(cfg.pcap_filename);
  }

  if (cfg.replay_write_enable) {
    printf("[Sniffer] Writing replay to: %s\n", cfg.replay_filename);
    replay_open(cfg.replay_filename);
  }
  
  int fd = sc_bus_connect();
  if (fd < 0) {return 1;}
  
  while (1) {
    bus_packet_t pkt;
    if (sc_bus_receive(fd, &pkt) == 0){
      if (cfg.filter_enabled && pkt.frame.can_id != cfg.filter_can_id) { continue; }
      
      print_frame(&pkt.frame);

      if (cfg.pcap_write_enabled) { pcap_write(&pkt.frame, pkt.timestamp_ns); }
      if (cfg.replay_write_enable) { replay_write(&pkt.frame); }
    }
    usleep(10);
  }
  close(fd);
  if (cfg.pcap_write_enabled){ pcap_close(); }
  if (cfg.replay_write_enable){ replay_close(); }
  return 0;
}
