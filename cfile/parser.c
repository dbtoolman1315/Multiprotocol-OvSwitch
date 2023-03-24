include "parser.h


"static inline ALWAYS_INLINE ovs_be16
parse_ethertype(const void **datap, size_t *sizep)
{
    const struct llc_snap_header *llc;
    ovs_be16 proto;

    proto = *(ovs_be16 *) data_pull(datap, sizep, sizeof proto);
    if (OVS_LIKELY(ntohs(proto) >= ETH_TYPE_MIN)) {
        return proto;
    }

    if (OVS_UNLIKELY(*sizep < sizeof *llc)) {
        return htons(FLOW_DL_TYPE_NONE);
    }

    llc = *datap;
    if (OVS_UNLIKELY(llc->llc.llc_dsap != LLC_DSAP_SNAP
                     || llc->llc.llc_ssap != LLC_SSAP_SNAP
                     || llc->llc.llc_cntl != LLC_CNTL_SNAP
                     || memcmp(llc->snap.snap_org, SNAP_ORG_ETHERNET,
                               sizeof llc->snap.snap_org))) {
        return htons(FLOW_DL_TYPE_NONE);
    }

    data_pull(datap, sizep, sizeof *llc);

    if (OVS_LIKELY(ntohs(llc->snap.snap_type) >= ETH_TYPE_MIN)) {
        return llc->snap.snap_type;
    }

    return htons(FLOW_DL_TYPE_NONE);
}

bool
parse_nsh(const void **datap, size_t *sizep, struct ovs_key_nsh *key)
{
    const struct nsh_hdr *nsh = (const struct nsh_hdr *) *datap;
    uint8_t version, length, flags, ttl;

    if (OVS_UNLIKELY(*sizep < NSH_BASE_HDR_LEN)) {
        return false;
    }

    version = nsh_get_ver(nsh);
    flags = nsh_get_flags(nsh);
    length = nsh_hdr_len(nsh);
    ttl = nsh_get_ttl(nsh);

    if (OVS_UNLIKELY(length > *sizep || version != 0)) {
        return false;
    }

    key->flags = flags;
    key->ttl = ttl;
    key->mdtype = nsh->md_type;
    key->np = nsh->next_proto;
    key->path_hdr = nsh_get_path_hdr(nsh);

    switch (key->mdtype) {
        case NSH_M_TYPE1:
            if (length != NSH_M_TYPE1_LEN) {
                return false;
            }
            for (size_t i = 0; i < 4; i++) {
                key->context[i] = get_16aligned_be32(&nsh->md1.context[i]);
            }
            break;
        case NSH_M_TYPE2:

            if (length < NSH_BASE_HDR_LEN) {
                return false;
            }

            memset(key->context, 0, sizeof(key->context));
            break;
        default:

            memset(key->context, 0, sizeof(key->context));
            break;
    }

    data_pull(datap, sizep, length);

    return true;
}
size_t parser_ip2(void **datap, uint32_t *sizep, struct mf_ctx *mfp)
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
