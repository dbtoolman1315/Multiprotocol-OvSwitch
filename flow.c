struct flow{ 
         struct flow_tnl tunnel;
         ovs_be64 metadata;
         uint32_t regs[FLOW_N_REGS];
         uint32_t skb_priority;
         uint32_t pkt_mark;
         uint32_t dp_hash;

         union flow_in_port in_port;
         uint32_t recirc_id;
         uint8_t ct_state;
         uint8_t ct_nw_proto;
         uint16_t ct_zone;
         uint32_t ct_mark;
         ovs_be32 packet_type;
         ovs_u128 ct_label;
         uint32_t conj_id;
         ofp_port_t actset_output; 
     struct eth_addr dl_dst; 
     struct eth_addr dl_src;
     ovs_be16 dl_type;
     struct  stji  ji;
     ovs_be16  zhao;
     uint8_t  yang;
};
