#!/usr/bin/python
# -*- coding: utf-8 -*-

#
# Copyright (c) 2014 Qualcomm Technologies Inc.
#

import xml.etree.ElementTree
import argparse
import os
import sys
import array

cdb0_offset_index = 0xe
cdb0_size_index = cdb0_offset_index + 2
cdb1_offset_index = cdb0_size_index + 2
cdb1_size_index = cdb1_offset_index + 2
cdb2_offset_index = cdb1_size_index + 2
cdb2_size_index = cdb2_offset_index + 2
cdb3_offset_index = cdb2_size_index + 2
cdb3_size_index = cdb3_offset_index + 2


def process_cdb(root, cdbname, cdbstart, cdbsize, modcount, data):
    for elem in root.findall(cdbname+'/*'):
        if elem.get('index') is not None:
            if modcount == 0:
                print ''
            modcount += 1
            size = 0
            bigendian = False
            if elem.get('length') is not None:
                size = int(elem.get('length'))
            else:
                size = 4

            if elem.get('endian') is not None:
                if elem.get('endian') == 'big':
                    bigendian = True
                elif elem.get('endian') == 'little':
                    bigendian = False
                else:
                    raise Exception('endian attribute should have only two values: bigendian or littleendian')

            index = int(elem.get('index'))

            if index >= cdbsize:
                raise Exception('Write index (%s)  greater than block size (%s)' % (
                    str(index), str(cdbsize)))
            else:
                index += cdbstart

            value = 0
            if elem.text.strip()[0] == '0' and elem.text.strip()[1] == 'x':
                value = int(elem.text.strip(), 16)
            else:
                value = int(elem.text.strip())

            for i in range(0, size):
                if bigendian:
                    data[index + i] = (value >> (8 * i)) & 0xFF
                else:
                    data[index + (size - i - 1)] = (value >> (8 * i)) & 0xFF
            print '\t' + elem.tag + ' value changed to ' + elem.text.strip() + ' at ' + str(index)

    return modcount

def parse_update(binfile, xmlfile, outfile):
    tree = xml.etree.ElementTree.parse(xmlfile)
    root = tree.getroot()
    cdb1 = root.findall('cdb1')
    modcount = 0
    if len(cdb1) != 1:
        print 'There should be only one cdb1 entry in the XML file'
        return False

    bsize = os.path.getsize(binfile)
    data = array.array('B', [0] * bsize)
    binfiledata = open(binfile, 'rb')
    for i in range(0, bsize):
        data[i] = ord(binfiledata.read(1))
    binfiledata.close()

    cdb1index = (data[cdb1_offset_index+1] << 8) | data[cdb1_offset_index]
    cdb1size = (data[cdb1_size_index+1] << 8) | data[cdb1_size_index]
    cdb2index = (data[cdb2_offset_index+1] << 8) | data[cdb2_offset_index]
    cdb2size = (data[cdb2_size_index+1] << 8) | data[cdb2_size_index]
    cdb3index = (data[cdb3_offset_index+1] << 8) | data[cdb3_offset_index]
    cdb3size = (data[cdb3_size_index+1] << 8) | data[cdb3_size_index]

    print '\tCDB1 index starts at: ' + str(cdb1index)
    print '\tCDB2 index starts at: ' + str(cdb2index)
    print '\tCDB3 index starts at: ' + str(cdb3index)
    print ''
    if (cdb1index+cdb1size) >= bsize:
        raise Exception('CDB1 offset is wrong. Corrupted cdt binary?')

    print ('\tMods:'),
    modcount += process_cdb(root, 'cdb1', cdb1index, cdb1size, modcount, data)
    modcount += process_cdb(root, 'cdb2', cdb2index, cdb2size, modcount, data)
    modcount += process_cdb(root, 'cdb3/*', cdb3index, cdb3size, modcount, data)

    if modcount == 0:
        print 'None'

    print ('\n\tWriting updated data file to file ' + outfile),
    binfiledata = open(outfile, 'wb')
    binfiledata.write(data)
    binfiledata.close()
    print 'done!\n'
    return True


def check_validity(binfile, xmlfile):
    bfile = open(binfile, 'r')
    data = bfile.read(3)
    bfile.close()
    if data != 'CDT':
        print 'Invalid CDT binary file ' + str(binfile)
        return False
    bsize = os.path.getsize(binfile)
    if bsize > (8 * 1024):
        print 'File size for ' + binfile + ' is too large'

    try:
        xml.etree.ElementTree.parse(xmlfile)
    except:
        print 'Invalid XML file ' + xmlfile
        return False

    return True


def main():
    print "\tCDT Modifier"
    parser = argparse.ArgumentParser()
    parser.add_argument("-c", "--cdt_bin", help="existing cdt binary")
    parser.add_argument("-o", "--cdt_outbin", help="name of output cdt binary")
    parser.add_argument("-x", "--cdt_modxml", help="CDT Modifier XML")
    args = parser.parse_args()

    if args.cdt_bin is None:
        raise Exception("CDT binary path needs to be given")

    if args.cdt_modxml is None:
        raise Exception("CDT Mod xml path needs to be given")

    try:
        with open(os.path.abspath(args.cdt_bin), 'r'):
            print '\n\tGiven CDT Binary ' + str(os.path.abspath(args.cdt_bin))
    except IOError:
        raise Exception('Given CDT Binary  ' + str(os.path.abspath(args.cdt_bin)) + ' not able open.'
                                                                                    ' Check if the file exists and is '
                                                                                    'writable!')

    try:
        with open(os.path.abspath(args.cdt_modxml), 'r'):
            print '\tGiven CDT modifier XML ' + str(os.path.abspath(args.cdt_modxml) + '\n')
    except IOError:
        raise Exception('Given CDT modifier XML ' + str(os.path.abspath(args.cdt_modxml)) + ' not able open for '
                                                                                            'reading. Check if the '
                                                                                            'file exists!')

    if check_validity(os.path.abspath(args.cdt_bin), os.path.abspath(args.cdt_modxml)) is False:
        raise Exception('Input files not correct. Cannot proceed')

    outfile = os.path.abspath(args.cdt_bin)
    if args.cdt_outbin is not None:
        outfile = os.path.abspath(args.cdt_outbin)

    if parse_update(os.path.abspath(args.cdt_bin), os.path.abspath(args.cdt_modxml), outfile) is False:
        raise Exception('Unable to parse and update binary data!')

    pass


if __name__ == '__main__':
    main()
