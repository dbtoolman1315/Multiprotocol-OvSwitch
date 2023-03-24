

sHash = '    hash = hash_add(hash, MINIFLOW_GET_TYPE(flow, fieldname));\n'

sHeader = 'uint32_t miniflow_hash()\n\
{\n\
	uint32_t hash = 0;\n\
	hash = hash_add(hash, MINIFLOW_GET_U32(flow, dl_dst));\n\
	hash = hash_add(hash, MINIFLOW_GET_U16(flow, (dl_dst<< 16)));\n\n\
	hash = hash_add(hash, MINIFLOW_GET_U32(flow, dl_src));\n\
	hash = hash_add(hash, MINIFLOW_GET_U16(flow, (dl_src << 16)));\n'

sTail = '    return hash_finish(hash, 42);\n\
}\n\n'
def sModHash(sWholeStr,name,size):
    dSizeToType = {8 : 'U8', 16 :'U16', 32 : 'U32'}   
    sWholeStr = sHash.replace('TYPE',dSizeToType[size])
    sWholeStr = sWholeStr.replace('fieldname', name)
    
    return sWholeStr

def vCreateHash(dHead,name):
    file_path = './cfile/hash.c'
    if(name != None):
        print(1)
    else:
        for fname,val in dHead.items():
            dHeadVal = val
        if('Hash' not in dHeadVal.keys()):
            return 
        else:
            sWholeStr = '\n'
            hashname = dHeadVal['Hash']
            for hn in hashname:
                sWholeStr = sWholeStr + sModHash(sWholeStr,hn,dHeadVal[hn]['size'])

        print(sWholeStr)
        with open(file_path, mode='a', encoding='utf-8') as file_obj:
            file_obj.write(sWholeStr)

def vHashInit():
    global sHeader
    #先打开parser.c
    file_path = './cfile/hash.c'
    with open(file_path, mode='w', encoding='utf-8') as file_obj:
            file_obj.write('include "hash.h"\n\n\n')
            file_obj.write(sHeader)

def vHashFinish():
    global sTail
    #先打开parser.c
    file_path = './cfile/hash.c'
    with open(file_path, mode='a', encoding='utf-8') as file_obj:
            file_obj.write(sTail)