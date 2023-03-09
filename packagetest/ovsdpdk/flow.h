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
#define bool _Bool

#define DL_TYPE_IP2 0xbbaa
#define DL_TYPE_IP3 0x9200
#define IP2_TYPE_TP1 0xa0
#define IP2_TYPE_TP2 0xa1

struct ovs_st24
{
    union
    {
        uint8_t eb[3];
    };
};

struct eth_addr
{
    union
    {
        uint8_t ea[6];
        ovs_be16 be16[3];
    };
};


struct flow
{
    /* L2 */
    struct eth_addr dl_dst;//1
    struct eth_addr dl_src;
    ovs_be16 dl_type;//2
    uint8_t pad1[2];

    /* L3 */
    ovs_be32 ip2_dst;//3
    ovs_be32 ip2_src;
    uint8_t ip2_info1;//4
    uint8_t ip2_type;
    ovs_be16 ip2_info2;
    struct ovs_st24 ip2_info3;
    uint8_t pad2;

    ovs_be16 ip3_dst;//5
    ovs_be16 ip3_src;
    uint8_t ip3_info1;
    uint8_t ip3_info2;
    ovs_be16 ip3_type;

    /* L4 */
    ovs_be16 tp1_dst;//6
    ovs_be16 tp1_src;
    ovs_be16 tp1_info2;
    ovs_be16 tp1_type;
    uint8_t tp1_info1;//7
    uint8_t pad3[7];

    uint8_t tp2_dst;//8
    uint8_t tp2_src;
    ovs_be16 tp2_info2;
    ovs_be32 tp2_info1;
    ovs_be16 tp2_type;//9
    uint8_t pad4[6];
};

#define DIV_ROUND_UP(X, Y) (((X) + ((Y)-1)) / (Y))
#define MAP_1 (map_t)1
#define MAP_T_BITS (sizeof(map_t) * 8)
#define FLOW_U64S (sizeof(struct flow) / sizeof(uint64_t))

#define FLOWMAP_UNITS DIV_ROUND_UP(FLOW_U64S, MAP_T_BITS)

struct flowmap
{
    map_t bits[FLOWMAP_UNITS];
};

struct mf_ctx
{
    struct flowmap map;
    uint64_t *data;
    uint64_t *const end;
};

struct miniflow
{
    struct flowmap map;
    /* Followed by:
     *     uint64_t values[n];
     * where 'n' is miniflow_n_values(miniflow). */
};

extern void miniflow_extract();
#endif