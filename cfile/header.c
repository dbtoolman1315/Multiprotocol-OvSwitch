 struct steth_dst {
        union {
            uint8_t ea[6     ];
            ovs_be16 be16[3     ];
         };
    };
struct steth_src {
        union {
            uint8_t ea[6     ];
            ovs_be16 be16[3     ];
         };
    };
struct stip2_info3
        union {
            uint8_t ea[3     ];
            ovs_be16 be16[1     ];
         };
    };
