# Multiprotocol-OvSwitch
一个支持多协议的软件交换机
# YAML TO C

## main.yml

```yaml
Layer2 : 
  - eth
Layer3 : 
  - ip
Layer4 :
  - myself
Layer5 :
  - null
#定义Layer 每层的协议根据选择填写
```

## 每层的具体内容

### eth.yml

```yaml
Ethernet :
  - dstAddr :
    - size : 48
    - offset : 0

  - srcAddr :
    - size : 48
    - offset : 6
    
  - etherType :
    - size : 16
    - offset : 12
```

![](E:\学习\SDN\T4P4S\yaml\图片\4.png)

![](E:\学习\SDN\T4P4S\yaml\图片\5.png)



## python转换代码

```python
#通过main.yml中的每层的头的名称找到对应的文件
import yaml
import json

with open('./main.yml', 'r', encoding = 'utf-8') as f:
    result = yaml.load(f.read(), loader = FullLoader)
print(result)

aLayer = ['Layer2', 'Layer3', 'Layer4', 'Layer5']

for la in aLayer:
    Layeri = result[la]
    for name in Layeri:
        if(None != name):
            newname = './' + name + '.yml'
            print(newname)
            with open(newname,'r', encoding = 'utf-8') as f:
                Li_result = yaml.load(f.read(), loader = FullLoader)
                json_str=json.dumps(Li_result)
                print(Li_result)
                file_path = 'tsts.c'
                with open(file_path, mode='a', encoding='utf-8') as file_obj:
                    file_obj.write(json_str)
         else:
            continue
```

![](E:\学习\SDN\T4P4S\yaml\图片\6.jpg)

## 需要生成或者添加的部分

### 1.flow以及与flow相关的数据结构的重新生成

这就需要在yaml文件填写好数据的类型，如ovs_be32，对于现有类型不能满足的，需要构造出结构体

```c
struct eth_addr {
    union {
        uint8_t ea[6];
        ovs_be16 be16[3];
    };
};
```

![](E:\学习\SDN\T4P4S\yaml\图片\1.png)

### 2.每个头对应的Parser

![](E:\学习\SDN\T4P4S\yaml\图片\2.png)

### 3.每个头对应的hash函数

![](E:\学习\SDN\T4P4S\yaml\图片\3.png)

### 4.miniflow(最重要)

按照TCP/IP协议栈的逻辑按顺序调用Parser然后填充miniflow

所以yaml文件的表就要写好协议的顺序，并且为每一个头配一个标签，指向下一个要解析的是什么头(如ether_type)

除非每一次配置文件都为按顺序写好的头，例如：- datatype : ovs_be32；

![image-20230227200704914](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20230227200704914.png)

## 生成FLow

分为两种情况

1.在ovs-dpdk已经存在的头，如eth ip tcp等，对于这种情况，可以直接复制ovs-dpdk中的代码作为字符串，通过字典这一数据类型，可以快速查找，然后将flow的具体内容输出到flow.c中。

2.自定义的头，这就需要Python代码生成，首先判断字节数，如果超出了最大的类型，就需要定义结构体类型。如果没有超过就去查找sHeaderStruct字符串然后替换相应的字符，再输出到flow.c中。

生成的代码存放在flow.c 以及header.c中

存在的问题

1.字节对齐问题.（非常重要）

2.重复协议问题.

## 生成Parser以及miniflow

需要仿照解析eth的函数生成

```c
{
    /* Link layer. */
    //判断长度是不是定义的eth数据类型的长度
    ASSERT_SEQUENTIAL(dl_dst, dl_src);

    //需要特别生成
    miniflow_push_macs(mf, dl_dst, data);

    /* VLAN */
    union flow_vlan_hdr vlans[FLOW_MAX_VLAN_HEADERS];
    size_t num_vlans = parse_vlan(&data, &size, vlans);

    dl_type = parse_ethertype(&data, &size);
    miniflow_push_be16(mf, dl_type, dl_type);
    miniflow_pad_to_64(mf, dl_type);
    if (num_vlans > 0) {
        miniflow_push_words_32(mf, vlans, vlans, num_vlans);
    }

 }
```

如果自定义协议是Myself，就要生成对应的miniflow_push_Myselfs函数

```c
#define miniflow_push_macs(MF, FIELD, VALUEP)                       \
    miniflow_push_macs_(MF, offsetof(struct flow, FIELD), VALUEP)
    
#define miniflow_push_macs_(MF, OFS, VALUEP)                    \
{                                                               \
    miniflow_set_maps(MF, (OFS) / 8, 2);                        \
    memcpy(MF.data, (VALUEP), 2 * ETH_ADDR_LEN);                \
    MF.data += 1;                   /* First word only. */      \
}
```

Myself_parser函数需要data_pull()函数的调用，不知道解析出的数据存在了哪了？ miniflow里边？

```c
ovs_be16 proto;

proto = *(ovs_be16 *) data_pull(datap, sizep, sizeof proto);
    
static inline const void *
data_pull(const void **datap, size_t *sizep, size_t size)
{
    const char *data = *datap;
    *datap = data + size;
    *sizep -= size;
    return data;
}
```

```c
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
```
