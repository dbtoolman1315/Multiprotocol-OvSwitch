#pragma once
#ifndef FLOW_H
#define FLOW_H


typedef unsigned long long uint64, uint64_t, ovs_be64, u64;
typedef long long int64, int64_t;
typedef unsigned int uint32, uint32_t, ovs_be32, u32;
typedef unsigned short uint16, uint16_t, ovs_be16, u16;
typedef unsigned char uint8, uint8_t, u8;
typedef uint64 __u64, __be64;
typedef uint32 __u32, __be32;
typedef uint16 __u16, __be16;
typedef uint8 __u8;
typedef unsigned long long map_t;


struct eth_addr {
    union {
        uint8_t ea[6];
        ovs_be16 be16[3];
    };
};

struct stji {
    union {
        uint8_t ea[6];
        ovs_be16 be16[3];
    };
};

struct flow {
 
    struct eth_addr dl_dst;
    struct eth_addr dl_src;
    ovs_be16 dl_type;
    struct  stji  ji;
    ovs_be16  zhao;
    uint8_t  yang;
    uint8_t pad[2];
};
#define DIV_ROUND_UP(X, Y) (((X) + ((Y) - 1)) / (Y))
#define MAP_1 (map_t)1
#define MAP_T_BITS (sizeof(map_t) * 8)
#define FLOW_U64S (sizeof(struct flow) / sizeof(uint64_t))

#define FLOWMAP_UNITS DIV_ROUND_UP(FLOW_U64S, MAP_T_BITS)

struct flowmap {
    map_t bits[FLOWMAP_UNITS];
};

struct mf_ctx {
    struct flowmap map;
    uint64_t* data;
    uint64_t* const end;
};


struct miniflow {
    struct flowmap map;
    /* Followed by:
     *     uint64_t values[n];
     * where 'n' is miniflow_n_values(miniflow). */
};


extern void extract();
#endif