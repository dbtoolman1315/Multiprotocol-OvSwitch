#通过main.yml中的每层的头的名称找到对应的文件
import yaml
import createflow
import createparser
import createmfext
import createhash

with open('./main.yml', 'r', encoding = 'utf-8') as f:
    result = yaml.load(f.read(), Loader=yaml.FullLoader)
print(result)

aLayer = ['Layer2', 'Layer3', 'Layer4', 'Layer5']
#判断是否为存在的协议
aExistHeader = ['ipv4','ipv6','tcp','udp','tcmp']
Ttype = {}
Ttype_size = {}

createflow.vFlowInit()
createparser.vParserInit()
createhash.vHashInit()
padsize = 0
for la in aLayer:
    Layeri = result[la]
    for name in Layeri:
        if(None != name):
            #存在的协议直接copy
            if(name in aExistHeader):
                padsize = createflow.vCreateFlow(None,name)
                createparser.vCreateParser(None,name,padsize)
            #新的协议再去生成
            else:
                newname = './header_describe/' + name + '.yml'
                print(newname)
                with open(newname,'r', encoding = 'utf-8') as f:
                    Li_result = yaml.load(f.read(), Loader=yaml.FullLoader)
                    print(Li_result)
                    padsize = createflow.vCreateFlow(Li_result,None)
                    createparser.vCreateParser(Li_result,None,padsize)    
                    
            with open('./header_describe/'+name+'.yml','r', encoding = 'utf-8') as f:
                    Li_result = yaml.load(f.read(), Loader=yaml.FullLoader)
                    #生成miniflow_extract  
                    createmfext.Ttype_get(Li_result,name,Ttype,Ttype_size)
                    #生成hash
                    createhash.vCreateHash(Li_result,None)
createmfext.create_extract(Ttype,Ttype_size)

createflow.vFlowEnd()
createhash.vHashFinish()


