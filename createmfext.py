import yaml
import json
import createflow
import createparser
align = "    "


def get_if(tp_name, value, cnt):
    return align*cnt + 'if (' + tp_name + ' == ' + value + ');\n'

def get_elif(tp_name, value, cnt):
    return align*cnt + 'else if (' + tp_name + ' == ' + value + ');\n'

def get_parser(name, size, cnt):
    name_ = name[0:len(name)-len('_type')]
    return align*cnt + size + ' ' + name + ' = ' + 'parser_' + name_ + ('(&data, &size, &mf);\n')


def extract_init(fo):
        fo.write(
            '\n\nvoid miniflow_extract(void *packet,struct miniflow *dst)\n'
            + "{\n"
            + align+'const void *data = packet;\n'
            + align+'uint32_t size = \n'
            + align+'uint64_t *values = miniflow_values(dst);\n'
        )
        
        
        
def Ttype_get(dHead,name,Ttype,Ttype_size):
    dSizeToType = {8: 'uint8_t', 16: 'ovs_be16', 32: 'ovs_be32'}
    file_path = './extract.c'

    for key, val in dHead.items():
        dHeadVal = val
        #print(dHeadVal)
        Ttype[name+'_type'] = dHeadVal.get('Type')
        Ttype_size[name+'_type'] = dSizeToType[(dHeadVal.get(name+'_type'))['size']] 
        


def defineinit(Ttype):
    with open('./extract.c', 'w', encoding='utf-8') as fo:
        for key,value in Ttype.items():
            if (value != None):
                for key_, value_ in value.items():
                    fo.write('#define ' + key_ + ' ' + value_ +'\n')

        
def iteration (typename, dtype, Ttype,Ttype_size,fo,cnt):
    fo.write(get_parser(typename,Ttype_size[typename],cnt))
    if (dtype != None):
        i=0
        for key, val in dtype:
            if(i==0):
                fo.write(get_if(typename,key,cnt) + align*cnt + '{\n')
                typename_ = key[1-(len(key)-len(typename)):]+'_type'
                if (Ttype[typename_]!=None):
                    dtype_ = Ttype[typename_].items()
                else:
                    dtype_ = Ttype[typename_]
                iteration (typename_, dtype_, Ttype, Ttype_size,fo,cnt+1)
                fo.write(cnt*align+'}\n')
            else:
                if(i<len(Ttype[typename])):
                    fo.write(get_elif('eth_type',key,cnt) + align*cnt + '{\n')   
                    typename_ = key[1-(len(key)-len(typename)):]+'_type'
                    if (Ttype[typename_]!=None):
                        dtype_ = Ttype[typename_].items()
                    else:
                        dtype_ = Ttype[typename_]
                    iteration (typename_, dtype_, Ttype, Ttype_size,fo,cnt+1)
                    fo.write(cnt*align+'}\n')
            i=i+1    

                

def create_extract(Ttype,Ttype_size):
    start = 'eth_type'
    fo = open('./extract.c', 'a', encoding='utf-8')
    defineinit(Ttype)
    extract_init(fo)
    iteration (start, Ttype[start].items(),Ttype,Ttype_size,fo,1)    
            
            
            
        
    
    
