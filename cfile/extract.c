#define ETH_TYPE_ip2 0xbbaa
#define ETH_TYPE_ip3 0x9200
#define ip2_TYPE_tp1 0xa0
#define ip2_TYPE_tp2 0xa1
#define ip2_TYPE_tp1 0xa0
#define ip2_TYPE_tp2 0xa1


void miniflow_extract(void *packet,struct miniflow *dst)
{
    const void *data = packet;
    uint32_t size = 
    uint64_t *values = miniflow_values(dst);
    ovs_be16 eth_type = parser_eth(&data, &size, &mf);
    if (eth_type == ETH_TYPE_ip2);
    {
        uint8_t ip2_type = parser_ip2(&data, &size, &mf);
        if (ip2_type == ip2_TYPE_tp1);
        {
            ovs_be16 tp1_type = parser_tp1(&data, &size, &mf);
        }
        else if (eth_type == ip2_TYPE_tp2);
        {
            ovs_be16 tp2_type = parser_tp2(&data, &size, &mf);
        }
    }
    else if (eth_type == ETH_TYPE_ip3);
    {
        ovs_be16 ip3_type = parser_ip3(&data, &size, &mf);
        if (ip3_type == ip2_TYPE_tp1);
        {
            ovs_be16 tp1_type = parser_tp1(&data, &size, &mf);
        }
        else if (eth_type == ip2_TYPE_tp2);
        {
            ovs_be16 tp2_type = parser_tp2(&data, &size, &mf);
        }
    }
