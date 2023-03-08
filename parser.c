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
