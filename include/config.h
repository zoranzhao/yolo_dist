#ifndef CONFIG__H
#define CONFIG__H


#define PORTNO 11111 //Service for job stealing and sharing
#define SMART_GATEWAY 11112 //Service for a smart gateway 
#define START_CTRL 11113 //Control the start and stop of a service
#define SRV "10.145.80.46"

#define GATEWAY "10.157.89.51"//"192.168.4.1"
#define AP "192.168.4.1"

#define PINK0    "192.168.4.16"
#define BLUE0    "192.168.4.14"
#define ORANGE0  "192.168.4.15"

#define PINK1    "192.168.4.4"
#define BLUE1    "192.168.4.9"
#define ORANGE1  "192.168.4.8"

#define CLI_NUM 6
#define ACT_CLI 4


#define DEBUG_DIST 0
#define STAGES 16
#define PARTITIONS_W 4
#define PARTITIONS_H 1
#define PARTITIONS 4
#define THREAD_NUM 1


#endif
