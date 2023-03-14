
#define miniflow_push_myselfs(MF, FIELD, VALUEP)                       \
    miniflow_push_myself_(MF, offsetof(struct flow, FIELD), VALUEP)
    
#define miniflow_push_myself_(MF, OFS, VALUEP)                    \
{                                                               \
    miniflow_set_maps(MF, (OFS) / 8, 2);                        \
    memcpy(MF.data, (VALUEP), 6+2);                \
    MF.data += 1;                   /* First word only. */      \
}

void *vpMyselfParser(const void **datap, size_t *sizep)
{
    miniflow_push_myselfs(mf, ji, data);
    data_pull(datap, sizep,6+2);

    uint8 ucyang = *(uint8 *)data_pull(datap,sizep,1);
    miniflow_push_be16(mf, yang, yang);

    //pad
    miniflow_pad_to_64(mf, dl_type);
    return &ucyang
}
