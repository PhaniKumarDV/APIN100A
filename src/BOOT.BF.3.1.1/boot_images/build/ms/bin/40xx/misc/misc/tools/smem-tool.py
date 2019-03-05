#!/usr/bin/env python
#
# Copyright (c) 2013-2015 Qualcomm Atheros, Inc.
#
import sys
import struct

from xml.etree import ElementTree as ETree
from collections import namedtuple
from collections import OrderedDict

AllocInfo = namedtuple("AllocInfo", "allocated offset size reserved")
alloc_info_fmt = "<IIII"

TInfo = namedtuple("TInfo", "index size")
align8 = lambda x: ((x + 7) & ~0x7)

def tinfo(index, size=0):
    return TInfo(index, size)

type_info_list = OrderedDict([
    ("SMEM_PROC_COMM", tinfo(0, 4 * 16)),
    ("SMEM_VERSION_INFO", tinfo(3, 32 * 4)),
    ("SMEM_HEAP_INFO", tinfo(1, 16)),
    ("SMEM_ALLOCATION_TABLE", tinfo(2, 512 * 16)),
    ("SMEM_HW_RESET_DETECT", tinfo(4, 8)),
    ("SMEM_AARM_WARM_BOOT", tinfo(5, 4)),
    ("SMEM_DIAG_ERR_MESSAGE", tinfo(6, 0xC8)),
    ("SMEM_SPINLOCK_ARRAY", tinfo(7, 8 * 4)),
    ("SMEM_MEMORY_BARRIER_LOCATION", tinfo(8, 4)),
    ("SMEM_AARM_PARTITION_TABLE", tinfo(9)),
    ("SMEM_HW_SW_BUILD_ID", tinfo(137)),
    ("SMEM_USABLE_RAM_PARTITION_TABLE", tinfo(402)),
    ("SMEM_BOOT_INFO_FOR_APPS", tinfo(418)),
    ("SMEM_BOOT_FLASH_TYPE", tinfo(478)),
    ("SMEM_BOOT_FLASH_INDEX", tinfo(479)),
    ("SMEM_BOOT_FLASH_CHIP_SELECT", tinfo(480)),
    ("SMEM_BOOT_FLASH_BLOCK_SIZE", tinfo(481)),
    ("SMEM_BOOT_FLASH_DENSITY", tinfo(482)),
])

index_to_type = dict((v.index,k) for k, v in type_info_list.iteritems())

SMEM_FIRST_VALID_TYPE = "SMEM_SPINLOCK_ARRAY"
SMEM_LAST_VALID_TYPE = "SMEM_BOOT_FLASH_BLOCK_SIZE"
SMEM_MAX_SIZE = 512
SMEM_LAST_STATIC_TYPE = "SMEM_MEMORY_BARRIER_LOCATION"
SMEM_LAST_STATIC_INDEX = type_info_list[SMEM_LAST_STATIC_TYPE].index

def debug(msg):
    print msg

def pack_array_node(array):
    data = []
    fmt = []

    length = int(array.attrib.get("length", 4))

    for entry in array.findall("entry"):
        value = pack_node(entry, pad=False)
        fmt.append("%ds" % len(value))
        data.append(value)

    entry_len = len(value)
    inited = len(array.findall("entry"))
    pad_len = (length - inited) * entry_len

    fmt.append("%ds" % pad_len)
    data.append("\0" * pad_len)

    fmt = "".join(fmt)
    return struct.pack(fmt, *data)

def pack_node(node, pad=True):
    data = []
    fmt = []

    for child in node.findall("*"):
        length = int(child.attrib.get("length", 4))
        dtype = child.attrib.get("type", "uint")

        if dtype == "uint":
            fmt.append("I")
            data.append(int(child.text, 0))
        elif dtype == "string":
            fmt.append("%ds" % length)
            data.append(child.text)
        elif dtype == "array":
            array = pack_array_node(child)
            fmt.append("%ds" % len(array))
            data.append(array)
        else:
            error("unsupported data type '%s'" % dtype)

    fmt = "".join(fmt)
    packed = struct.pack(fmt, *data)

    if pad:
        packed_len = len(packed)
        pad_len = align8(packed_len) - packed_len
        pad_bytes = "\0" * pad_len
    else:
        pad_bytes = ""

    return packed + pad_bytes

def pack_xml(smem_xml):
    smem_data = [None] * SMEM_MAX_SIZE

    tree = ETree.parse(smem_xml)
    root = tree.getroot()
    for child in root.iter("data"):
        try:
            type_name = child.attrib["type"]
        except KeyError:
            error("SMEM type unspecified")
        try:
            index = type_info_list[type_name].index
        except KeyError:
            error("unsupported SMEM type '%s'" % type_name)

        smem_data[index] = pack_node(child)

    return smem_data

def calc_static_size():
    size = 0
    for i in range(SMEM_LAST_STATIC_INDEX + 1):
        smem_type = index_to_type[i]
        type_info = type_info_list[smem_type]
        size += type_info.size

    return size

def dump_dynamic(smem_fp, smem_data):
    for i in range(SMEM_LAST_STATIC_INDEX + 1, SMEM_MAX_SIZE):
        data = smem_data[i]
        if data != None:
            smem_fp.write(data)

def dump_alloc_info(smem_fp, smem_data):
    # FIXME: Offset calculation is incorrect for static data

    offset = calc_static_size()
    alloc_info_list = []
    for i, data in enumerate(smem_data):
        allocated = 0
        size = 0
        aoffset = 0
        if data != None:
            allocated = 1
            size = len(data)
            aoffset = offset

        ainfo = AllocInfo(allocated, aoffset, size, 0)
        ainfo_packed = struct.pack(alloc_info_fmt, *ainfo)

        alloc_info_list.append(ainfo_packed)
        offset += size

    return "".join(alloc_info_list)

def dump_static(smem_fp, smem_data):
    # Dump in type_info_list order, since that is order in which the
    # data is layed out, in memory, not in index order.

    for i, smem_type in enumerate(type_info_list.iterkeys()):
        data = smem_data[i]
        type_info = type_info_list[smem_type]

        if smem_type == "SMEM_ALLOCATION_TABLE":
            data = dump_alloc_info(smem_fp, smem_data)

        elif data == None:
            data = "\0" * type_info.size

        smem_fp.write(data)

        if smem_type == SMEM_LAST_STATIC_TYPE:
            break

def dump_all(smem_bin, smem_data):
    smem_fp = open(smem_bin, "w")

    dump_static(smem_fp, smem_data)
    dump_dynamic(smem_fp, smem_data)

    smem_fp.close()

def smem_tool(smem_xml, smem_bin):
    smem_data = pack_xml(smem_xml)
    dump_all(smem_bin, smem_data)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        usage(1)

    smem_tool(sys.argv[1], sys.argv[2])

