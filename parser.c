static inline ALWAYS_INLINE ovs_be16
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
static inline bool
parse_ipv6_ext_hdrs__(const void **datap, size_t *sizep, uint8_t *nw_proto,
                      uint8_t *nw_frag,
                      const struct ovs_16aligned_ip6_frag **frag_hdr)
{
    *frag_hdr = NULL;
    while (1) {
        if (OVS_LIKELY((*nw_proto != IPPROTO_HOPOPTS)
                       && (*nw_proto != IPPROTO_ROUTING)
                       && (*nw_proto != IPPROTO_DSTOPTS)
                       && (*nw_proto != IPPROTO_AH)
                       && (*nw_proto != IPPROTO_FRAGMENT))) {

            return true;
        }

        if (OVS_UNLIKELY(*sizep < 8)) {
            return false;
        }

        if ((*nw_proto == IPPROTO_HOPOPTS)
            || (*nw_proto == IPPROTO_ROUTING)
            || (*nw_proto == IPPROTO_DSTOPTS)) {

            const struct ip6_ext *ext_hdr = *datap;

            if (OVS_UNLIKELY(!data_try_pull(datap, sizep,
                                            (ext_hdr->ip6e_len + 1) * 8))) {
                return false;
            }
        } else if (*nw_proto == IPPROTO_AH) {

            const struct ip6_ext *ext_hdr = *datap;
            *nw_proto = ext_hdr->ip6e_nxt;
            if (OVS_UNLIKELY(!data_try_pull(datap, sizep,
                                            (ext_hdr->ip6e_len + 2) * 4))) {
                return false;
            }
        } else if (*nw_proto == IPPROTO_FRAGMENT) {
            *frag_hdr = *datap;

            *nw_proto = (*frag_hdr)->ip6f_nxt;
            if (!data_try_pull(datap, sizep, sizeof **frag_hdr)) {
                return false;
            }


            if ((*frag_hdr)->ip6f_offlg != htons(0)) {
                *nw_frag = FLOW_NW_FRAG_ANY;
                if (((*frag_hdr)->ip6f_offlg & IP6F_OFF_MASK) != htons(0)) {
                    *nw_frag |= FLOW_NW_FRAG_LATER;
                    *nw_proto = IPPROTO_FRAGMENT;
                    return true;
                }
            }
        }
    }
}

bool
parse_ipv6_ext_hdrs(const void **datap, size_t *sizep, uint8_t *nw_proto,
                    uint8_t *nw_frag,
                    const struct ovs_16aligned_ip6_frag **frag_hdr)
{
    return parse_ipv6_ext_hdrs__(datap, sizep, nw_proto, nw_frag,
                                 frag_hdr);
}

static inline bool
ipv4_sanity_check(const struct ip_header *nh, size_t size,
                  int *ip_lenp, uint16_t *tot_lenp)
{
    int ip_len;
    uint16_t tot_len;

    if (OVS_UNLIKELY(size < IP_HEADER_LEN)) {
        COVERAGE_INC(miniflow_extract_ipv4_pkt_too_short);
        return false;
    }
    ip_len = IP_IHL(nh->ip_ihl_ver) * 4;

    if (OVS_UNLIKELY(ip_len < IP_HEADER_LEN || size < ip_len)) {
        COVERAGE_INC(miniflow_extract_ipv4_pkt_len_error);
        return false;
    }

    tot_len = ntohs(nh->ip_tot_len);
    if (OVS_UNLIKELY(tot_len > size || ip_len > tot_len ||
                size - tot_len > UINT16_MAX)) {
        COVERAGE_INC(miniflow_extract_ipv4_pkt_len_error);
        return false;
    }

    *ip_lenp = ip_len;
    *tot_lenp = tot_len;

    return true;
}

static inline uint8_t
ipv4_get_nw_frag(const struct ip_header *nh)
{
    uint8_t nw_frag = 0;

    if (OVS_UNLIKELY(IP_IS_FRAGMENT(nh->ip_frag_off))) {
        nw_frag = FLOW_NW_FRAG_ANY;
        if (nh->ip_frag_off & htons(IP_FRAG_OFF_MASK)) {
            nw_frag |= FLOW_NW_FRAG_LATER;
        }
    }

    return nw_frag;
}

static inline bool
ipv6_sanity_check(const struct ovs_16aligned_ip6_hdr *nh, size_t size)
{
    uint16_t plen;

    if (OVS_UNLIKELY(size < sizeof *nh)) {
        COVERAGE_INC(miniflow_extract_ipv6_pkt_too_short);
        return false;
    }

    plen = ntohs(nh->ip6_plen);
    if (OVS_UNLIKELY(plen + IPV6_HEADER_LEN > size)) {
        COVERAGE_INC(miniflow_extract_ipv6_pkt_len_error);
        return false;
    }

    if (OVS_UNLIKELY(size - (plen + IPV6_HEADER_LEN) > UINT16_MAX)) {
        COVERAGE_INC(miniflow_extract_ipv6_pkt_len_error);
        return false;
    }

    return true;
}

static void
dump_invalid_packet(struct dp_packet *packet, const char *reason)
{
    static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 5);
    struct ds ds = DS_EMPTY_INITIALIZER;
    size_t size;

    if (VLOG_DROP_DBG(&rl)) {
        return;
    }
    size = dp_packet_size(packet);
    ds_put_hex_dump(&ds, dp_packet_data(packet), size, 0, false);
    VLOG_DBG("invalid packet for %s: port %"PRIu32", size %"PRIuSIZE"
%s",
             reason, packet->md.in_port.odp_port, size, ds_cstr(&ds));
    ds_destroy(&ds);
}
#define miniflow_push_myself_(MF, OFS, VALUEP)                    \ 
{                                                               \ 
    miniflow_set_maps(MF, (OFS) / 8, 1);                        \ 
     memcpy(MF.data, (VALUEP), 8);                \ 
    MF.data += 1;                   /* First word only. */      \ 
} 
#define miniflow_push_myselfs(MF, FIELD, VALUEP)                       \  
    miniflow_push_myself_(MF, offsetof(struct flow, FIELD), VALUEP)


uint8_t vpMyselfParser    (const void **datap, size_t *sizep)
{
    miniflow_push_myselfs     (mf, ji,     data);
    data_pull(datap, sizep, 6    );
    data_pull(datap, sizep, 2    );

    yang = *(uint8_t  *)data_pull(datap,sizep,1);
    miniflow_push_be16(mf, yang, yang);

    //pad
    miniflow_pad_to_64(mf, dl_type);
    return yang
}

