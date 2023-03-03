import createflow


def findstr(sStr,fStr):
    locationName = sStr.find(fStr)
    return locationName
sfparser = 'void *vpMyselfParser         (const void **datap, size_t *sizep)\n\
{\n\
    miniflow_push_myselfs(mf, ji, data);\n\
    data_pull(datap, sizep,6+2);\n\
\n\
    uint8 ucyang = *(uint8 *)data_pull(datap,sizep,1);\n\
    miniflow_push_be16(mf, yang, yang);\n\
\n\
    //pad\n\
    miniflow_pad_to_64(mf, dl_type);\n\
    return &ucyang\n\
}\n'
def vModFunName(name):
    global sfparser
    fnameLocation =  sfparser.find('MyselfParser')     
    sfparser = list(sfparser)
    
    sname = name + 'Parser'
    size = len(sname)
    for i in range(size + 6):
        sfparser[i + fnameLocation] = sname[i]
    sfparser = ''.join(sfparser)
    #sHeaderStruct = str(sHeaderStruct)
    file_path = 'parser.c'
    with open(file_path, mode='a', encoding='utf-8') as file_obj:
        file_obj.write(sfparser)

def vCreateParser(dHead,name):
    global sfparser
    if(name != None):
        print(1)
    else:
        for fname,val in dHead.items():
            dHeadVal = val
            vModFunName(fname)
        for key, val in dHeadVal.items():
            for key1,size in val.items():
                fnameLocation = createflow.findstr(sfparser,'MyselfParser')
