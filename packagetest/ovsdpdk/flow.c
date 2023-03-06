#include "flow.h"
#include "string.h"

#define size_t uint32_t

flowmap_set(struct flowmap* fm, size_t idx, unsigned int n_bits)
{
    map_t n_bits_mask = (MAP_1 << n_bits) - 1;
    size_t unit = idx / MAP_T_BITS;

    idx %= MAP_T_BITS;

    fm->bits[unit] |= n_bits_mask << idx;
    /* The seemingly unnecessary bounds check on 'unit' is a workaround for a
     * false-positive array out of bounds error by GCC 4.9. */
    if (unit + 1 < FLOWMAP_UNITS && idx + n_bits > MAP_T_BITS) {
        /* 'MAP_T_BITS - idx' bits were set on 'unit', set the remaining
         * bits from the next unit. */
        fm->bits[unit + 1] |= n_bits_mask >> (MAP_T_BITS - idx);
    }
}

#define FLOWMAP_EMPTY_INITIALIZER { { 0 } }
//miniflow
//typedef unsigned int size_t;

#define offsetof(type, member) \
    ((size_t)((char *)&(((type *)0)->member) - (char *)0))


#define miniflow_set_map(MF, OFS)            \
    {                                        \
    flowmap_set(&MF.map, (OFS), 1);          \
}

#define miniflow_set_maps(MF, OFS, N_WORDS)                     \
{                                                               \
    size_t ofs = (OFS);                                         \
    size_t n_words = (N_WORDS);                                 \
                                                                \
    flowmap_set(&MF.map, ofs, n_words);                         \
}

#define miniflow_push_myself_(MF, OFS, VALUEP)                    \
{                                                               \
    miniflow_set_maps(MF, (OFS) / 8, 2);                        \
    memcpy(MF.data, (VALUEP), 6+2);                \
    MF.data += 1;                   /* First word only. */      \
}
#define miniflow_push_myselfs(MF, FIELD, VALUEP)                       \
    miniflow_push_myself_(MF, offsetof(struct flow, FIELD), VALUEP)

#define miniflow_push_uint16_(MF, OFS, VALUE)   \
    if ((OFS) % 8 == 0) {                       \
    miniflow_set_map(MF, OFS / 8);              \
    }                                           \
}
#define miniflow_push_be16_(MF, OFS, VALUE)                     \
    miniflow_push_uint16_(MF, OFS, (OVS_FORCE uint16_t)VALUE);

#define miniflow_push_be16(MF, FIELD, VALUE)                        \
    miniflow_push_be16_(MF, offsetof(struct flow, FIELD), VALUE)







//struct
#define MAP_T_BITS (sizeof(map_t) * 8)

#define DIV_ROUND_UP(X, Y) (((X) + ((Y) - 1)) / (Y))
#define FLOWMAP_UNITS DIV_ROUND_UP(FLOW_U64S, MAP_T_BITS)





static inline uint64_t* miniflow_values(struct miniflow* mf)
{
    return (uint64_t*)(mf + 1);
}

static inline const void*
data_pull(const void** datap, size_t* sizep, size_t size)
{
    const char* data = *datap;
    *datap = data + size;
    *sizep -= size;
    return data;
}


void* vpMyselfParser(const void* datap, uint32_t* sizep, struct mf_ctx mf)
{

    miniflow_push_myselfs(mf, ji, datap);
    data_pull(datap, sizep, 6 + 2);

    uint8_t ucyang = *(uint8_t*)data_pull(&datap, sizep, 1);

}

uint8 packet[] = {1,2,3,4,5,6,7,8,9};
void extract()
{
    struct {
        struct miniflow mf;
        uint64_t buf[FLOW_U64S];
    } m;

    const void* data = packet;
    uint32_t size = sizeof(packet);

    uint64_t* values = miniflow_values(&m.mf);
    struct mf_ctx mf = { FLOWMAP_EMPTY_INITIALIZER, values,
                         values + FLOW_U64S };

    vpMyselfParser(data, &size, mf);
}



