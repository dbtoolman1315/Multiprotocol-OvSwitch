dEth = {'Ethernet': {'dstAddr': {'size': 48}, 'srcAddr': {'size': 48}, 'etherType': {'size': 16}}}


EthFlow = 'struct eth_addr dl_dst; \n \
    struct eth_addr dl_src;\n \
    ovs_be16 dl_type;\n \
    uint8_t pad1[2];\n'

Ipv4Flow = 'ovs_be32 nw_src;\n \
    ovs_be32 nw_dst;\n \
    ovs_be32 ct_nw_src;\n\
    ovs_be32 ct_nw_dst;\n'

Ipv6Flow = 'struct in6_addr ipv6_src;\n\
    struct in6_addr ipv6_dst;\n\
    struct in6_addr ct_ipv6_src;\n\
    struct in6_addr ct_ipv6_dst;\n\
    ovs_be32 ipv6_label;\n\
    struct in6_addr nd_target;\n'

Ipv4HelpFlow = 'uint8_t nw_frag;\n\
    uint8_t nw_tos;\n\
    uint8_t nw_ttl;\n\
    uint8_t nw_proto;\n\
    ovs_be32 pad2;\n'


TcpUdpStcpFlow = 'ovs_be16 tp_src;\n\
    ovs_be16 tp_dst;\n\
    ovs_be16 ct_tp_src;\n\
    ovs_be16 ct_tp_dst;\n\
    ovs_be32 igmp_group_ip4;\n\
    ovs_be32 pad3;\n'

# 填充计数
pad = 4
# 存在协议对应的字符串
dExistName = {'eth': EthFlow, 'ipv4': Ipv4Flow, 'ipv6': Ipv6Flow, 'tcp': TcpUdpStcpFlow, 'udp': TcpUdpStcpFlow,
              'icmp': TcpUdpStcpFlow}

# 长度超出的创建结构体
def creatheader(num, name):
    sHeaderStruct ='struct XXX       {\n\
        union {\n\
            uint8_t ea[ucsize];\n\
            ovs_be16 be16[sssize];\n \
        };\n\
    };\n'
    #找位置，替换字符串，先转换成list，因为str不可修改
    locationName = sHeaderStruct.find('XXX')
    sname = 'st' + str(name)
    namesize = len(sname)
    loaction1 = sHeaderStruct.find('ucsize]')
    loaction2 = sHeaderStruct.find('sssize]')
    #print(loaction1)
    sHeaderStruct = list(sHeaderStruct)

    num = (num / 8)
    num = int(num)
    snum = str(num)
    numsize = len(snum)

    num16 = num / 2
    num16 = int(num16)
    snum16 = str(num16)
    num16size = len(snum16)

    for i in range(15):
        if(i < numsize):
            sHeaderStruct[i + loaction1] = snum[i]
        elif (i < 6):
            sHeaderStruct[i + loaction1] =' '

        if(i < num16size):
            sHeaderStruct[i + loaction2] = snum16[i]
        elif (i < 6):
            sHeaderStruct[i + loaction2] =' '
        if(i < namesize):
            sHeaderStruct[i + locationName] = sname[i]
    sHeaderStruct = ''.join(sHeaderStruct)
    #sHeaderStruct = str(sHeaderStruct)
    file_path = './cfile/header.c'
    with open(file_path, mode='a', encoding='utf-8') as file_obj:
        file_obj.write(sHeaderStruct)
    print(sHeaderStruct)

    return sname


#main
def vCreateFlow(dHead, name):
    global pad
    #已存在协议
    if name is not None:
        file_path = './cfile/flow.c'
        stwrite = dExistName[name]
        #ip需要特殊处理一下
        if name == 'ipv4':
            with open(file_path, mode='a', encoding='utf-8') as file_obj:
                file_obj.write('     ' + stwrite + '     ' + Ipv4HelpFlow)
        else:
            with open(file_path, mode='a', encoding='utf-8') as file_obj:
                file_obj.write('     ' + stwrite)
        return 
    #新协议
    else:
        dSizeToType = {8: 'uint8_t', 16: 'ovs_be16', 32: 'ovs_be32'}
        padsize = 0
        file_path = './cfile/flow.c'
        for key, val in dHead.items():
            dHeadVal = val

        if('Type' in val.keys()):
            val.pop('Type')
        if('Hash' in val.keys()):
            val.pop('Hash')

        for key, val in dHeadVal.items():
            for key1, size in val.items():
                padsize += size
                if size in dSizeToType.keys():
                    with open(file_path, mode='a', encoding='utf-8') as file_obj:
                        file_obj.write('     ' + dSizeToType[size] + '  ' + key + ';\n')
                    #print(dSizeToType[size] + '  ' + key + ',\n')
                else:
                    sname = creatheader(size,key)
                    swrite = 'struct  ' + sname +'  ' + key
                    with open(file_path, mode='a', encoding='utf-8') as file_obj:
                        file_obj.write('     ' + swrite + ';\n')
        # 填充到64bit
        if padsize % 64:
            inx = 64 - padsize % 64
            if inx in dSizeToType:
                with open(file_path, mode='a', encoding='utf-8') as file_obj:
                    file_obj.write('     ' + dSizeToType[inx] + ' pad' + str(pad) + ';\n')
            else:
                with open(file_path, mode='a', encoding='utf-8') as file_obj:
                    inx = inx // 8

                    file_obj.write('     ' + dSizeToType[8] + ' pad' + str(pad) + '[' + str(inx) + ']' + ';\n')
            pad += 1

        return padsize
        #with open(file_path, mode='a', encoding='utf-8') as file_obj:
        #               file_obj.write('\n')





def vFlowInit():
    sFirstLine = 'struct flow{ \n \
        struct flow_tnl tunnel;\n \
        ovs_be64 metadata;\n \
        uint32_t regs[FLOW_N_REGS];\n \
        uint32_t skb_priority;\n \
        uint32_t pkt_mark;\n \
        uint32_t dp_hash;\n\n \
        union flow_in_port in_port;\n \
        uint32_t recirc_id;\n \
        uint8_t ct_state;\n \
        uint8_t ct_nw_proto;\n \
        uint16_t ct_zone;\n \
        uint32_t ct_mark;\n \
        ovs_be32 packet_type;\n \
        ovs_u128 ct_label;\n \
        uint32_t conj_id;\n \
        ofp_port_t actset_output; \n'

    file_path = './cfile/flow.c'
    with open(file_path, mode='w', encoding='utf-8') as file_obj:
        file_obj.write(sFirstLine)

    with open('./cfile/header.c', mode='w', encoding='utf-8') as test:
        test.write('include "header.h"\n\n')

def vFlowEnd():
    file_path = './cfile/flow.c'
    with open(file_path, mode='a', encoding='utf-8') as file_obj:
                    file_obj.write('};\n')