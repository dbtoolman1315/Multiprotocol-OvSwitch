#include "flow.h"
#include "string.h"
//#include "printf.h"
//#include <stddef.h>

#define size_t uint32_t
#define ETH_ADDR_LEN 6
#define MINIFLOW_ASSERT(X)
#define OVS_FORCE
#define offsetof(type, member) \
    ((size_t)((char *)&(((type *)0)->member) - (char *)0))


#define miniflow_set_map(MF, OFS)               \
    {                                           \
        ASSERT_FLOWMAP_NOT_SET(&MF.map, (OFS)); \
        flowmap_set(&MF.map, (OFS), 1);         \
    }

#define miniflow_set_maps(MF, OFS, N_WORDS)                     \
    {                                                           \
        size_t ofs = (OFS);                                     \
        size_t n_words = (N_WORDS);                             \
                                                                \
        MINIFLOW_ASSERT(n_words &&MF.data + n_words <= MF.end); \
        ASSERT_FLOWMAP_NOT_SET(&MF.map, ofs);                   \
        flowmap_set(&MF.map, ofs, n_words);                     \
    }

#define ASSERT_FLOWMAP_NOT_SET(FM, IDX)                                 \
    {                                                                   \
        MINIFLOW_ASSERT(!((FM)->bits[(IDX) / MAP_T_BITS] &              \
                          (MAP_MAX << ((IDX) % MAP_T_BITS))));          \
        for (size_t i = (IDX) / MAP_T_BITS + 1; i < FLOWMAP_UNITS; i++) \
        {                                                               \
            MINIFLOW_ASSERT(!(FM)->bits[i]);                            \
        }                                                               \
    }

static inline bool
flowmap_is_set(const struct flowmap *fm, size_t idx)
{
    return (fm->bits[idx / MAP_T_BITS] & (MAP_1 << (idx % MAP_T_BITS))) != 0;
}

#define miniflow_assert_in_map(MF, OFS)              \
    MINIFLOW_ASSERT(flowmap_is_set(&MF.map, (OFS))); \
    ASSERT_FLOWMAP_NOT_SET(&MF.map, (OFS) + 1)

#define miniflow_push_uint8_(MF, OFS, VALUE)             \
    {                                                    \
        MINIFLOW_ASSERT(MF.data < MF.end);               \
                                                         \
        if ((OFS) % 8 == 0)                              \
        {                                                \
            miniflow_set_map(MF, OFS / 8);               \
            *(uint8_t *)MF.data = VALUE;                 \
        }                                                \
        else if ((OFS) % 8 == 7)                         \
        {                                                \
            miniflow_assert_in_map(MF, OFS / 8);         \
            *((uint8_t *)MF.data + 7) = VALUE;           \
            MF.data++;                                   \
        }                                                \
        else                                             \
        {                                                \
            miniflow_assert_in_map(MF, OFS / 8);         \
            *((uint8_t *)MF.data + ((OFS) % 8)) = VALUE; \
        }                                                \
    }

#define miniflow_push_uint16_(MF, OFS, VALUE)    \
    {                                            \
        MINIFLOW_ASSERT(MF.data < MF.end);       \
                                                 \
        if ((OFS) % 8 == 0)                      \
        {                                        \
            miniflow_set_map(MF, OFS / 8);       \
            *(uint16_t *)MF.data = VALUE;        \
        }                                        \
        else if ((OFS) % 8 == 2)                 \
        {                                        \
            miniflow_assert_in_map(MF, OFS / 8); \
            *((uint16_t *)MF.data + 1) = VALUE;  \
        }                                        \
        else if ((OFS) % 8 == 4)                 \
        {                                        \
            miniflow_assert_in_map(MF, OFS / 8); \
            *((uint16_t *)MF.data + 2) = VALUE;  \
        }                                        \
        else if ((OFS) % 8 == 6)                 \
        {                                        \
            miniflow_assert_in_map(MF, OFS / 8); \
            *((uint16_t *)MF.data + 3) = VALUE;  \
            MF.data++;                           \
        }                                        \
    }

#define miniflow_push_uint32_(MF, OFS, VALUE)    \
    {                                            \
        MINIFLOW_ASSERT(MF.data < MF.end);       \
                                                 \
        if ((OFS) % 8 == 0)                      \
        {                                        \
            miniflow_set_map(MF, OFS / 8);       \
            *(uint32_t *)MF.data = VALUE;        \
        }                                        \
        else if ((OFS) % 8 == 4)                 \
        {                                        \
            miniflow_assert_in_map(MF, OFS / 8); \
            *((uint32_t *)MF.data + 1) = VALUE;  \
            MF.data++;                           \
        }                                        \
    }

#define miniflow_push_words_32_(MF, OFS, VALUEP, N_WORDS)           \
    {                                                               \
        miniflow_set_maps(MF, (OFS) / 8, DIV_ROUND_UP(N_WORDS, 2)); \
        memcpy(MF.data, (VALUEP), (N_WORDS) * sizeof(uint32_t));    \
        MF.data += DIV_ROUND_UP(N_WORDS, 2);                        \
        if ((N_WORDS)&1)                                            \
        {                                                           \
            *((uint32_t *)MF.data - 1) = 0;                         \
        }                                                           \
    }

#define miniflow_push_macs_(MF, OFS, VALUEP)         \
    {                                                \
        miniflow_set_maps(MF, (OFS) / 8, 2);         \
        memcpy(MF.data, (VALUEP), 2 * ETH_ADDR_LEN); \
        MF.data += 1; /* First word only. */         \
    }

#define MEMBER_SIZEOF(STRUCT, MEMBER) (sizeof(((STRUCT *)NULL)->MEMBER))

#define OFFSETOFEND(STRUCT, MEMBER) \
    (offsetof(STRUCT, MEMBER) + MEMBER_SIZEOF(STRUCT, MEMBER))

#define miniflow_push_uint8(MF, FIELD, VALUE) \
    miniflow_push_uint8_(MF, offsetof(struct flow, FIELD), VALUE)

#define miniflow_push_be16_(MF, OFS, VALUE) \
    miniflow_push_uint16_(MF, OFS, (OVS_FORCE uint16_t)VALUE);

#define miniflow_push_be16(MF, FIELD, VALUE) \
    miniflow_push_be16_(MF, offsetof(struct flow, FIELD), VALUE)

#define miniflow_push_be32_(MF, OFS, VALUE) \
    miniflow_push_uint32_(MF, OFS, (OVS_FORCE uint32_t)(VALUE))

#define miniflow_push_be32(MF, FIELD, VALUE) \
    miniflow_push_be32_(MF, offsetof(struct flow, FIELD), VALUE)

#define miniflow_push_macs(MF, FIELD, VALUEP) \
    miniflow_push_macs_(MF, offsetof(struct flow, FIELD), VALUEP)

#define miniflow_pad_to_64_(MF, OFS)                              \
    {                                                             \
        MINIFLOW_ASSERT((OFS) % 8 != 0);                          \
        miniflow_assert_in_map(MF, OFS / 8);                      \
                                                                  \
        memset((uint8_t *)MF.data + (OFS) % 8, 0, 8 - (OFS) % 8); \
        MF.data++;                                                \
    }

#define miniflow_pad_to_64(MF, FIELD) \
    miniflow_pad_to_64_(MF, OFFSETOFEND(struct flow, FIELD))

#define FLOWMAP_EMPTY_INITIALIZER \
    {                             \
        {                         \
            0                     \
        }                         \
    }



#define miniflow_set_map(MF, OFS)       \
    {                                   \
        flowmap_set(&MF.map, (OFS), 1); \
    }

#define miniflow_set_maps(MF, OFS, N_WORDS) \
    {                                       \
        size_t ofs = (OFS);                 \
        size_t n_words = (N_WORDS);         \
                                            \
        flowmap_set(&MF.map, ofs, n_words); \
    }

#define MAP_T_BITS (sizeof(map_t) * 8)
#define DIV_ROUND_UP(X, Y) (((X) + ((Y)-1)) / (Y))
#define FLOWMAP_UNITS DIV_ROUND_UP(FLOW_U64S, MAP_T_BITS)

flowmap_set(struct flowmap *fm, size_t idx, unsigned int n_bits)
{
    map_t n_bits_mask = (MAP_1 << n_bits) - 1;
    size_t unit = idx / MAP_T_BITS;

    idx %= MAP_T_BITS;

    fm->bits[unit] |= n_bits_mask << idx;
    /* The seemingly unnecessary bounds check on 'unit' is a workaround for a
     * false-positive array out of bounds error by GCC 4.9. */
    if (unit + 1 < FLOWMAP_UNITS && idx + n_bits > MAP_T_BITS)
    {
        /* 'MAP_T_BITS - idx' bits were set on 'unit', set the remaining
         * bits from the next unit. */
        fm->bits[unit + 1] |= n_bits_mask >> (MAP_T_BITS - idx);
    }
}

static inline uint64_t *miniflow_values(struct miniflow *mf)
{
    return (uint64_t *)(mf + 1);
}

static inline const void *
data_pull(const void **datap, size_t *sizep, size_t size)
{
    const char *data = *datap;
    *datap = data + size;
    *sizep -= size;
    return data;
}

/*
void* vpMyselfParser(const void* datap, uint32_t* sizep, struct mf_ctx mf)
{

    miniflow_push_myselfs(mf, ji, datap);
    data_pull(datap, sizep, 6 + 2);

    uint8_t ucyang = *(uint8_t*)data_pull(&datap, sizep, 1);

}*/


ovs_be16 parser_tp1(const void **datap, uint32_t *sizep, struct mf_ctx *mfp)
{
    
}

ovs_be16 parser_tp2(const void **datap, uint32_t *sizep, struct mf_ctx *mfp)
{
    miniflow_push_uint8((*mfp), tp2_dst, *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));
    
    miniflow_push_uint8((*mfp), tp2_src, *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));

    miniflow_push_be16((*mfp), tp2_info2, *(ovs_be16 *)data_pull(datap, sizep, sizeof(ovs_be16)));

    miniflow_push_be32((*mfp), tp2_info1, *(ovs_be32 *)data_pull(datap, sizep, sizeof(ovs_be32)));

    ovs_be16 type = *(ovs_be16 *)data_pull(datap, sizep, sizeof(ovs_be16));
    miniflow_push_be16((*mfp), tp2_type, type);

    miniflow_pad_to_64((*mfp), tp2_type);
}


uint8_t parser_ip2(void **datap, uint32_t *sizep, struct mf_ctx *mfp)
{
    miniflow_push_be32((*mfp), ip2_dst, *(ovs_be32 *)data_pull(datap, sizep, sizeof(ovs_be32)));

    miniflow_push_be32((*mfp), ip2_src, *(ovs_be32 *)data_pull(datap, sizep, sizeof(ovs_be32)));

    miniflow_push_uint8((*mfp), ip2_info1, *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));

    uint8_t type = *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t));
    miniflow_push_uint8((*mfp), ip2_type, type);

    miniflow_push_be16((*mfp), ip2_info2, *(ovs_be16 *)data_pull(datap, sizep, sizeof(ovs_be16)));

    miniflow_push_uint8((*mfp), ip2_info3.eb[0], *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));
    miniflow_push_uint8((*mfp), ip2_info3.eb[1], *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));
    miniflow_push_uint8((*mfp), ip2_info3.eb[2], *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));

    miniflow_pad_to_64((*mfp), ip2_info3);

    

    return type;
}

uint8_t parser_ip3(void **datap, uint32_t *sizep, struct mf_ctx *mfp)
{
    
}

ovs_be16 parser_eth(const void **datap, uint32_t *sizep, struct mf_ctx *mfp)
{
    miniflow_push_macs((*mfp), dl_dst, *datap);
    data_pull(datap, sizep, ETH_ADDR_LEN * 2);

    ovs_be16 type = *(ovs_be16 *)data_pull(datap, sizep, sizeof(ovs_be16));
    miniflow_push_be16((*mfp), dl_type, type);

    miniflow_pad_to_64((*mfp), dl_type);
    return type;
}

void miniflow_extract(void *packet, struct miniflow *dst)
{

    const void *data = packet;
    uint32_t size = 39;
    uint64_t *values = miniflow_values(dst);
    struct mf_ctx mf = {FLOWMAP_EMPTY_INITIALIZER, values,
                        values + FLOW_U64S};

    ovs_be16 dl_type = parser_eth(&data, &size, &mf);

    if (dl_type == DL_TYPE_IP2)
    {
        uint8_t ip2_type = parser_ip2(&data, &size, &mf);
        if (ip2_type == IP2_TYPE_TP1)
        {
            uint16_t tp2_type = parser_tp1(&data, &size, &mf);
        }
        else if(ip2_type == IP2_TYPE_TP2)
        {
            uint16_t tp2_type = parser_tp2(&data, &size, &mf);
        }
    }
    else if(dl_type == DL_TYPE_IP3)
    {
        uint16_t ip3_type = parser_ip3(&data, &size, &mf);
    }

   // printf("%s", "\na debug info");
}
