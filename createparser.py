import createflow
import math

parse_mpls = 'static inline int\n\
parse_mpls(const void **datap, size_t *sizep)\n\
{\n\
    const struct mpls_hdr *mh;\n\
    int count = 0;\n\
\n\
    while ((mh = data_try_pull(datap, sizep, sizeof *mh))) {\n\
        count++;\n\
        if (mh->mpls_lse.lo & htons(1 << MPLS_BOS_SHIFT)) {\n\
            break;\n\
        }\n\
    }\n\
    return MIN(count, FLOW_MAX_MPLS_LABELS);\n\
}\n'

parse_vlan = 'static inline ALWAYS_INLINE size_t\n\
parse_vlan(const void **datap, size_t *sizep, union flow_vlan_hdr *vlan_hdrs)\n\
{\n\
    const ovs_be16 *eth_type;\n\
\n\
    data_pull(datap, sizep, ETH_ADDR_LEN * 2);\n\
\n\
    eth_type = *datap;\n\
\n\
    size_t n;\n\
    for (n = 0; eth_type_vlan(*eth_type) && n < flow_vlan_limit; n++) {\n\
        if (OVS_UNLIKELY(*sizep < sizeof(ovs_be32) + sizeof(ovs_be16))) {\n\
            break;\n\
        }\n\
\n\
        memset(vlan_hdrs + n, 0, sizeof(union flow_vlan_hdr));\n\
        const ovs_16aligned_be32 *qp = data_pull(datap, sizep, sizeof *qp);\n\
        vlan_hdrs[n].qtag = get_16aligned_be32(qp);\n\
        vlan_hdrs[n].tci |= htons(VLAN_CFI);\n\
        eth_type = *datap;\n\
    }\n\
    return n;\n\
}\n'

parse_eth = 'static inline ALWAYS_INLINE ovs_be16\n\
parse_ethertype(const void **datap, size_t *sizep)\n\
{\n\
    const struct llc_snap_header *llc;\n\
    ovs_be16 proto;\n\
\n\
    proto = *(ovs_be16 *) data_pull(datap, sizep, sizeof proto);\n\
    if (OVS_LIKELY(ntohs(proto) >= ETH_TYPE_MIN)) {\n\
        return proto;\n\
    }\n\
\n\
    if (OVS_UNLIKELY(*sizep < sizeof *llc)) {\n\
        return htons(FLOW_DL_TYPE_NONE);\n\
    }\n\
\n\
    llc = *datap;\n\
    if (OVS_UNLIKELY(llc->llc.llc_dsap != LLC_DSAP_SNAP\n\
                     || llc->llc.llc_ssap != LLC_SSAP_SNAP\n\
                     || llc->llc.llc_cntl != LLC_CNTL_SNAP\n\
                     || memcmp(llc->snap.snap_org, SNAP_ORG_ETHERNET,\n\
                               sizeof llc->snap.snap_org))) {\n\
        return htons(FLOW_DL_TYPE_NONE);\n\
    }\n\
\n\
    data_pull(datap, sizep, sizeof *llc);\n\
\n\
    if (OVS_LIKELY(ntohs(llc->snap.snap_type) >= ETH_TYPE_MIN)) {\n\
        return llc->snap.snap_type;\n\
    }\n\
\n\
    return htons(FLOW_DL_TYPE_NONE);\n\
}\n\
\n\
bool\n\
parse_nsh(const void **datap, size_t *sizep, struct ovs_key_nsh *key)\n\
{\n\
    const struct nsh_hdr *nsh = (const struct nsh_hdr *) *datap;\n\
    uint8_t version, length, flags, ttl;\n\
\n\
    if (OVS_UNLIKELY(*sizep < NSH_BASE_HDR_LEN)) {\n\
        return false;\n\
    }\n\
\n\
    version = nsh_get_ver(nsh);\n\
    flags = nsh_get_flags(nsh);\n\
    length = nsh_hdr_len(nsh);\n\
    ttl = nsh_get_ttl(nsh);\n\
\n\
    if (OVS_UNLIKELY(length > *sizep || version != 0)) {\n\
        return false;\n\
    }\n\
\n\
    key->flags = flags;\n\
    key->ttl = ttl;\n\
    key->mdtype = nsh->md_type;\n\
    key->np = nsh->next_proto;\n\
    key->path_hdr = nsh_get_path_hdr(nsh);\n\
\n\
    switch (key->mdtype) {\n\
        case NSH_M_TYPE1:\n\
            if (length != NSH_M_TYPE1_LEN) {\n\
                return false;\n\
            }\n\
            for (size_t i = 0; i < 4; i++) {\n\
                key->context[i] = get_16aligned_be32(&nsh->md1.context[i]);\n\
            }\n\
            break;\n\
        case NSH_M_TYPE2:\n\
\n\
            if (length < NSH_BASE_HDR_LEN) {\n\
                return false;\n\
            }\n\
\n\
            memset(key->context, 0, sizeof(key->context));\n\
            break;\n\
        default:\n\
\n\
            memset(key->context, 0, sizeof(key->context));\n\
            break;\n\
    }\n\
\n\
    data_pull(datap, sizep, length);\n\
\n\
    return true;\n\
}\n'

parse_icmpv6 = 'static inline bool\n\
parse_icmpv6(const void **datap, size_t *sizep,\n\
             const struct icmp6_data_header *icmp6,\n\
             ovs_be32 *rso_flags, const struct in6_addr **nd_target,\n\
             struct eth_addr arp_buf[2], uint8_t *opt_type)\n\
{\n\
    if (icmp6->icmp6_base.icmp6_code != 0 ||\n\
        (icmp6->icmp6_base.icmp6_type != ND_NEIGHBOR_SOLICIT &&\n\
         icmp6->icmp6_base.icmp6_type != ND_NEIGHBOR_ADVERT)) {\n\
        return false;\n\
    }\n\
\n\
    arp_buf[0] = eth_addr_zero;\n\
    arp_buf[1] = eth_addr_zero;\n\
    *opt_type = 0;\n\
\n\
    *rso_flags = get_16aligned_be32(icmp6->icmp6_data.be32);\n\
\n\
    *nd_target = data_try_pull(datap, sizep, sizeof **nd_target);\n\
    if (OVS_UNLIKELY(!*nd_target)) {\n\
        return true;\n\
    }\n\
\n\
    while (*sizep >= 8) {\n\
        /* The minimum size of an option is 8 bytes, which also is\n\
         * the size of Ethernet link-layer options. */\n\
        const struct ovs_nd_lla_opt *lla_opt = *datap;\n\
        int opt_len = lla_opt->len * ND_LLA_OPT_LEN;\n\
\n\
        if (!opt_len || opt_len > *sizep) {\n\
            return true;\n\
        }\n\
\n\
\n\
        if (lla_opt->type == ND_OPT_SOURCE_LINKADDR && opt_len == 8) {\n\
            if (OVS_LIKELY(eth_addr_is_zero(arp_buf[0]))) {\n\
                arp_buf[0] = lla_opt->mac;\n\
\n\
                if (*opt_type == 0) {\n\
                    *opt_type = lla_opt->type;\n\
                }\n\
            } else {\n\
                goto invalid;\n\
            }\n\
        } else if (lla_opt->type == ND_OPT_TARGET_LINKADDR && opt_len == 8) {\n\
            if (OVS_LIKELY(eth_addr_is_zero(arp_buf[1]))) {\n\
                arp_buf[1] = lla_opt->mac;\n\
\n\
                if (*opt_type == 0) {\n\
                    *opt_type = lla_opt->type;\n\
                }\n\
            } else {\n\
                goto invalid;\n\
            }\n\
        }\n\
\n\
        if (OVS_UNLIKELY(!data_try_pull(datap, sizep, opt_len))) {\n\
            return true;\n\
        }\n\
    }\n\
    return true;\n\
\n\
invalid:\n\
    *nd_target = NULL;\n\
    arp_buf[0] = eth_addr_zero;\n\
    arp_buf[1] = eth_addr_zero;\n\
    return true;\n\
}\n'

parse_ip = 'static inline bool\n\
parse_ipv6_ext_hdrs__(const void **datap, size_t *sizep, uint8_t *nw_proto,\n\
                      uint8_t *nw_frag,\n\
                      const struct ovs_16aligned_ip6_frag **frag_hdr)\n\
{\n\
    *frag_hdr = NULL;\n\
    while (1) {\n\
        if (OVS_LIKELY((*nw_proto != IPPROTO_HOPOPTS)\n\
                       && (*nw_proto != IPPROTO_ROUTING)\n\
                       && (*nw_proto != IPPROTO_DSTOPTS)\n\
                       && (*nw_proto != IPPROTO_AH)\n\
                       && (*nw_proto != IPPROTO_FRAGMENT))) {\n\
\n\
            return true;\n\
        }\n\
\n\
        if (OVS_UNLIKELY(*sizep < 8)) {\n\
            return false;\n\
        }\n\
\n\
        if ((*nw_proto == IPPROTO_HOPOPTS)\n\
            || (*nw_proto == IPPROTO_ROUTING)\n\
            || (*nw_proto == IPPROTO_DSTOPTS)) {\n\
\n\
            const struct ip6_ext *ext_hdr = *datap;\n\
\n\
            if (OVS_UNLIKELY(!data_try_pull(datap, sizep,\n\
                                            (ext_hdr->ip6e_len + 1) * 8))) {\n\
                return false;\n\
            }\n\
        } else if (*nw_proto == IPPROTO_AH) {\n\
\n\
            const struct ip6_ext *ext_hdr = *datap;\n\
            *nw_proto = ext_hdr->ip6e_nxt;\n\
            if (OVS_UNLIKELY(!data_try_pull(datap, sizep,\n\
                                            (ext_hdr->ip6e_len + 2) * 4))) {\n\
                return false;\n\
            }\n\
        } else if (*nw_proto == IPPROTO_FRAGMENT) {\n\
            *frag_hdr = *datap;\n\
\n\
            *nw_proto = (*frag_hdr)->ip6f_nxt;\n\
            if (!data_try_pull(datap, sizep, sizeof **frag_hdr)) {\n\
                return false;\n\
            }\n\
\n\
\n\
            if ((*frag_hdr)->ip6f_offlg != htons(0)) {\n\
                *nw_frag = FLOW_NW_FRAG_ANY;\n\
                if (((*frag_hdr)->ip6f_offlg & IP6F_OFF_MASK) != htons(0)) {\n\
                    *nw_frag |= FLOW_NW_FRAG_LATER;\n\
                    *nw_proto = IPPROTO_FRAGMENT;\n\
                    return true;\n\
                }\n\
            }\n\
        }\n\
    }\n\
}\n\
\n\
bool\n\
parse_ipv6_ext_hdrs(const void **datap, size_t *sizep, uint8_t *nw_proto,\n\
                    uint8_t *nw_frag,\n\
                    const struct ovs_16aligned_ip6_frag **frag_hdr)\n\
{\n\
    return parse_ipv6_ext_hdrs__(datap, sizep, nw_proto, nw_frag,\n\
                                 frag_hdr);\n\
}\n\
\n\
static inline bool\n\
ipv4_sanity_check(const struct ip_header *nh, size_t size,\n\
                  int *ip_lenp, uint16_t *tot_lenp)\n\
{\n\
    int ip_len;\n\
    uint16_t tot_len;\n\
\n\
    if (OVS_UNLIKELY(size < IP_HEADER_LEN)) {\n\
        COVERAGE_INC(miniflow_extract_ipv4_pkt_too_short);\n\
        return false;\n\
    }\n\
    ip_len = IP_IHL(nh->ip_ihl_ver) * 4;\n\
\n\
    if (OVS_UNLIKELY(ip_len < IP_HEADER_LEN || size < ip_len)) {\n\
        COVERAGE_INC(miniflow_extract_ipv4_pkt_len_error);\n\
        return false;\n\
    }\n\
\n\
    tot_len = ntohs(nh->ip_tot_len);\n\
    if (OVS_UNLIKELY(tot_len > size || ip_len > tot_len ||\n\
                size - tot_len > UINT16_MAX)) {\n\
        COVERAGE_INC(miniflow_extract_ipv4_pkt_len_error);\n\
        return false;\n\
    }\n\
\n\
    *ip_lenp = ip_len;\n\
    *tot_lenp = tot_len;\n\
\n\
    return true;\n\
}\n\
\n\
static inline uint8_t\n\
ipv4_get_nw_frag(const struct ip_header *nh)\n\
{\n\
    uint8_t nw_frag = 0;\n\
\n\
    if (OVS_UNLIKELY(IP_IS_FRAGMENT(nh->ip_frag_off))) {\n\
        nw_frag = FLOW_NW_FRAG_ANY;\n\
        if (nh->ip_frag_off & htons(IP_FRAG_OFF_MASK)) {\n\
            nw_frag |= FLOW_NW_FRAG_LATER;\n\
        }\n\
    }\n\
\n\
    return nw_frag;\n\
}\n\
\n\
static inline bool\n\
ipv6_sanity_check(const struct ovs_16aligned_ip6_hdr *nh, size_t size)\n\
{\n\
    uint16_t plen;\n\
\n\
    if (OVS_UNLIKELY(size < sizeof *nh)) {\n\
        COVERAGE_INC(miniflow_extract_ipv6_pkt_too_short);\n\
        return false;\n\
    }\n\
\n\
    plen = ntohs(nh->ip6_plen);\n\
    if (OVS_UNLIKELY(plen + IPV6_HEADER_LEN > size)) {\n\
        COVERAGE_INC(miniflow_extract_ipv6_pkt_len_error);\n\
        return false;\n\
    }\n\
\n\
    if (OVS_UNLIKELY(size - (plen + IPV6_HEADER_LEN) > UINT16_MAX)) {\n\
        COVERAGE_INC(miniflow_extract_ipv6_pkt_len_error);\n\
        return false;\n\
    }\n\
\n\
    return true;\n\
}\n\
\n\
static void\n\
dump_invalid_packet(struct dp_packet *packet, const char *reason)\n\
{\n\
    static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 5);\n\
    struct ds ds = DS_EMPTY_INITIALIZER;\n\
    size_t size;\n\
\n\
    if (VLOG_DROP_DBG(&rl)) {\n\
        return;\n\
    }\n\
    size = dp_packet_size(packet);\n\
    ds_put_hex_dump(&ds, dp_packet_data(packet), size, 0, false);\n\
    VLOG_DBG("invalid packet for %s: port %"PRIu32", size %"PRIuSIZE"\n%s",\n\
             reason, packet->md.in_port.odp_port, size, ds_cstr(&ds));\n\
    ds_destroy(&ds);\n\
}\n'

parser_names = {'mpls': parse_mpls, 'vlan': parse_vlan, 'eth': parse_eth, 'ipv4': parse_ip,
                'ipv6': parse_ip, 'icmpv6': parse_icmpv6}

sfparser = 'datasize_t vpMyselfParser    (const void **datap, size_t *sizep)\n\
{\n\
    miniflow_push_myselfs     (mf, filedname,     data);\n\
    data_pull(datap, sizep, fieldsizeone    );\n\
    data_pull(datap, sizep, fieldsizetwo    );\n\
\n\
    indicatetype = *(datasize_t  *)data_pull(datap,sizep,1);\n\
    miniflow_push_be16(mf, yang, yang);\n\
\n\
    //pad\n\
    miniflow_pad_to_64(mf, dl_type);\n\
    return indicatetype\n\
}\n'

sfminiflow = '#define miniflow_push_myself_(MF, OFS, VALUEP)                    \ \n\
{                                                               \ \n\
    miniflow_set_maps(MF, (OFS) / 8, num64);                        \ \n \
    memcpy(MF.data, (VALUEP), bitosize);                \ \n\
    MF.data += 1;                   /* First word only. */      \ \n\
} \n\
#define miniflow_push_myselfs(MF, FIELD, VALUEP)                       \  \n\
    miniflow_push_myself_(MF, offsetof(struct flow, FIELD), VALUEP)\n\n\n'


def sModMiniflowNum(sWholeMiniflow, allsize):
    num64 = allsize / 64
    num64 = math.ceil(num64)
    snum64 = str(num64)
    sWholeMiniflow = sWholeMiniflow.replace('num64', snum64)
    num8 = allsize / 8
    num8 = int(num8)
    num8 = str(num8)
    sWholeMiniflow = sWholeMiniflow.replace('bitosize', num8)
    print(sWholeMiniflow)
    return sWholeMiniflow


def ReplaceStr(sReplaceName, sWholeStr, sYamlName, sAfterYamlName):
    fnameLocation = sfparser.find(sReplaceName)
    sWholeStr = list(sWholeStr)

    sname = sYamlName + sAfterYamlName
    size = len(sname)
    for i in range(size):
        sWholeStr[i + fnameLocation] = sname[i]
    sWholeStr = ''.join(sWholeStr)
    return sWholeStr


def sModVariableName(sWholeStr, sFieldName, xFieldSize, cnt, xHeaderSize):
    if (cnt == 1):
        sWholeStr = sWholeStr.replace('filedname', sFieldName)
    s = xFieldSize / 8
    s = int(s)
    s = str(s)
    dSizeToType = {8: 'uint8_t', 16: 'ovs_be16', 32: 'ovs_be32'}
    if (cnt == xHeaderSize):
        sWholeStr = sWholeStr.replace('indicatetype', sFieldName)
        sWholeStr = sWholeStr.replace('datasize_t', dSizeToType[xFieldSize])

    elif (cnt == 1):
        sWholeStr = sWholeStr.replace('fieldsizeone', s)

    elif (cnt == 2):
        sWholeStr = sWholeStr.replace('fieldsizetwo', s)

    print(sWholeStr)
    return sWholeStr


def sModFunName(name):
    global sfparser
    global sfminiflow
    sWholeStr = ReplaceStr('MyselfParser', sfparser, name, 'Parser')

    name = name.lower()
    sWholeStr = ReplaceStr('myselfs', sWholeStr, name, 's')

    sWholeMiniflow = sfminiflow.replace('myself', name)
    print(sWholeMiniflow)
    file_path = 'parser.c'
    # with open(file_path, mode='w', encoding='utf-8') as file_obj:
    # file_obj.write(sfparser)
    return sWholeStr, sWholeMiniflow


def vCreateParser(dHead, name):
    global sfparser
    if name is not None:
        file_path = 'parser.c'
        with open(file_path, mode='a', encoding='utf-8') as file_obj:
            file_obj.write(parser_names.get(name))
            print(parser_names.get(name))
    else:
        for fname, val in dHead.items():
            xHeaderSize = len(val)
            dHeadVal = val
            sWholeStr, sWholeMiniflow = sModFunName(fname)
        cnt = 0
        xAllSize = 0
        for fieldname, val in dHeadVal.items():
            cnt = cnt + 1
            for key1, size in val.items():
                xAllSize = xAllSize + size
                sWholeStr = sModVariableName(sWholeStr, fieldname, size, cnt, xHeaderSize)
                if (cnt == xHeaderSize - 1):
                    sWholeMiniflow = sModMiniflowNum(sWholeMiniflow, xAllSize)
        file_path = 'parser.c'
        with open(file_path, mode='a', encoding='utf-8') as file_obj:
            file_obj.write(sWholeMiniflow)
            file_obj.write(sWholeStr + '\n')


def vParserInit():
    file_path = 'parser.c'
    f = open(file_path, "r+")
    f.seek(0)
    f.truncate()
