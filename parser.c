include "parser.h


"size_t parser_ip2(void **datap, uint32_t *sizep, struct mf_ctx *mfp)
{ 
    miniflow_push_be32((*mfp), ip2_dst, *(ovs_be32 *)data_pull(datap, sizep, sizeof(ovs_be32)));

    miniflow_push_be32((*mfp), ip2_src, *(ovs_be32 *)data_pull(datap, sizep, sizeof(ovs_be32)));

    miniflow_push_uint8((*mfp), ip2_info1, *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));

    uint8_t rtype = *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t));
    miniflow_push_uint8((*mfp), ip2_type, rtype);

    miniflow_push_be16((*mfp), ip2_info2, *(ovs_be16 *)data_pull(datap, sizep, sizeof(ovs_be16)));

    miniflow_push_uint8((*mfp), ip2_info3.eb[0], *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));
    miniflow_push_uint8((*mfp), ip2_info3.eb[1], *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));
    miniflow_push_uint8((*mfp), ip2_info3.eb[2], *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));
    miniflow_pad_to_64((*mfp), ip2_info3);

    return rtype;
}
size_t parser_ip3(void **datap, uint32_t *sizep, struct mf_ctx *mfp)
{ 
    miniflow_push_be16((*mfp), ip3_dst, *(ovs_be16 *)data_pull(datap, sizep, sizeof(ovs_be16)));

    miniflow_push_be16((*mfp), ip3_src, *(ovs_be16 *)data_pull(datap, sizep, sizeof(ovs_be16)));

    miniflow_push_uint8((*mfp), ip3_info1, *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));

    miniflow_push_uint8((*mfp), ip3_info2, *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));

    ovs_be16 rtype = *(ovs_be16 *)data_pull(datap, sizep, sizeof(ovs_be16));
    miniflow_push_be16((*mfp), ip3_type, rtype);

    miniflow_pad_to_64((*mfp), ip3_type);

    return rtype;
}
size_t parser_tp1(void **datap, uint32_t *sizep, struct mf_ctx *mfp)
{ 
    miniflow_push_be16((*mfp), tp1_dst, *(ovs_be16 *)data_pull(datap, sizep, sizeof(ovs_be16)));

    miniflow_push_be16((*mfp), tp1_src, *(ovs_be16 *)data_pull(datap, sizep, sizeof(ovs_be16)));

    miniflow_push_be16((*mfp), tp1_info2, *(ovs_be16 *)data_pull(datap, sizep, sizeof(ovs_be16)));

    ovs_be16 rtype = *(ovs_be16 *)data_pull(datap, sizep, sizeof(ovs_be16));
    miniflow_push_be16((*mfp), tp1_type, rtype);

    miniflow_push_uint8((*mfp), tp1_info1, *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));

    miniflow_pad_to_64((*mfp), tp1_info1);

    return rtype;
}
size_t parser_tp2(void **datap, uint32_t *sizep, struct mf_ctx *mfp)
{ 
    miniflow_push_uint8((*mfp), tp2_dst, *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));

    miniflow_push_uint8((*mfp), tp2_src, *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));

    miniflow_push_be16((*mfp), tp2_info2, *(ovs_be16 *)data_pull(datap, sizep, sizeof(ovs_be16)));

    miniflow_push_be32((*mfp), tp2_info1, *(ovs_be32 *)data_pull(datap, sizep, sizeof(ovs_be32)));

    ovs_be16 rtype = *(ovs_be16 *)data_pull(datap, sizep, sizeof(ovs_be16));
    miniflow_push_be16((*mfp), tp2_type, rtype);

    miniflow_pad_to_64((*mfp), tp2_type);

    return rtype;
}
