void *vpMyselfParser         (const void **datap, size_t *sizep)
{
    miniflow_push_myselfs(mf, ji, data);
    data_pull(datap, sizep,6+2);

    uint8 ucyang = *(uint8 *)data_pull(datap,sizep,1);
    miniflow_push_be16(mf, yang, yang);

    //pad
    miniflow_pad_to_64(mf, dl_type);
    return &ucyang
}
