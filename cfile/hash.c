include "hash.h"


uint32_t miniflow_hash()
{
	uint32_t hash = 0;
	hash = hash_add(hash, MINIFLOW_GET_U32_MOF(flow, dl_dst));
	hash = hash_add(hash, MINIFLOW_GET_U16_MOF(flow, (dl_dst<< 16)));

	hash = hash_add(hash, MINIFLOW_GET_U32_MOF(flow, dl_src));
	hash = hash_add(hash, MINIFLOW_GET_U16_MOF(flow, (dl_src << 16)));

    hash = hash_add(hash, MINIFLOW_GET_U32_MOF(flow, ip2_dst));
    hash = hash_add(hash, MINIFLOW_GET_U32_MOF(flow, ip2_src));

    hash = hash_add(hash, MINIFLOW_GET_U16_MOF(flow, ip3_dst));
    hash = hash_add(hash, MINIFLOW_GET_U16_MOF(flow, ip3_type));

    hash = hash_add(hash, MINIFLOW_GET_U8_MOF(flow, tp2_dst));
    hash = hash_add(hash, MINIFLOW_GET_U8_MOF(flow, tp2_src));
    return hash_finish(hash, 42);
}

