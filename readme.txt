SMBIOS表结构中包含了厂商和设备名信息
BIOS start address：0xF0000
BIOS MAP LENGTH: 0xFFFF

Entry Point Structure解析:
00h: 4Bytes _SM_
0Bh-0Fh: 5Bytes _DMI_
....
16h: Struct Table Length
18h: Struct Table Address

system information Type1 structure
00h: 1(type)
04h: Manafacture
05h: Product Name

https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.6.0.pdf
******************协议文档
https://blog.csdn.net/wlswls1711/article/details/119945391
https://blog.csdn.net/weixin_45279063/article/details/118539468
******************字符串区域找到连续的0000H，就可以找到下一个Type，字符串区域紧随在格式区域后的一个区域，字符串区域的长度不是固定的
可参考以上三个链接
