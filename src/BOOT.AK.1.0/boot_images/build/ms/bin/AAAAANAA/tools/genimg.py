#!/usr/bin/python
# -*- coding: utf-8 -*-

#
# Copyright (c) 2013-2015 Qualcomm Technologies Inc.
#

import os
import ConfigParser
import subprocess
import errno
import argparse
import shutil
import sys

global sbl_path
configdir = 'config'
nor_tool = '$$/boot_images/core/bsp/tools/flash/nor_exe/build/nor_tool'
mbn_gen = \
    '$$/boot_images/core/storage/tools/nandbootmbn/nand_mbn_generator.py'
cdt_gen = '$$/boot_images/core/boot/secboot3/scripts/cdt_generator.py'
cdt_mod = '$$/tools/genimage/misc/cdt_mod.py'
smem_gen = '$$/tools/genimage/misc/smem-tool.py'
bootconfig_gen = '$$/boot_images/core/bsp/tools/flash/bootconfig_exe/build/bootconfig_tool'
cbreadme = '$$/tools/genimage/misc/README.txt'
cb_configs = '$$/tools/genimage/misc/cbconfig'
configfile = 'boardconfig'
nanduserpartition = 'nand-user-partition.bin'
noruserpartition = 'nor-user-partition.bin'
noruserpartition_v2 = 'nor-user-partition-v2.bin'
nand_raw_bootconfig='nand_raw_bootconfig.bin'
nor_raw_bootconfig='nor_raw_bootconfig.bin'
nand_raw_bootconfig1='nand_raw_bootconfig1.bin'
nor_raw_bootconfig1='nor_raw_bootconfig1.bin'
norplusnandusrpartition = 'norplusnand-user-partition.bin'
norplusemmcusrpartition = 'norplusemmc-user-partition.bin'
ref_cdt_xml = '$$/tools/genimage/config/AP148-cdt.xml'
ptool = '$$/boot_images/core/storage/tools/jsdcc/partition_load_pt/ptool.py'
msp = '$$/boot_images/core/storage/tools/jsdcc/partition_load_pt/msp.py'

smeminfobin = 'smem.bin'
cdtbin = 'cdt.mbn'
ssdbin = 'ssd.mbn'
bootconfigbin = 'bootconfig.bin'


def check_path(skipexport):
    global nor_tool
    global mbn_gen
    global cdt_gen
    global smem_gen
    global bootconfig_gen

    try:
        with open(nor_tool, 'r'):
            print 'nor_tool location: ' + nor_tool
    except IOError:
        print 'nor_tool not found. Have you built nor_tool_exe?'
        return -1

    try:
        with open(mbn_gen, 'r'):
            print 'nand_mbn_generator.py location: ' + mbn_gen
    except IOError:
        print 'nand_mbn_generator.py not found in (' + mbn_gen \
            + '). Incomplete source?'
        return -1

    if not skipexport:
        try:
            with open(cdt_gen, 'r'):
                print 'cdt_generator.py location: ' + cdt_gen
        except IOError:
            print 'cdt_generator.py not found. Incomplete source? Missed --skip_export option?'
            return -1

    if not skipexport:
        try:
            with open(ref_cdt_xml, 'r'):
                print '* Taking reference CDT xml : ' + ref_cdt_xml
        except IOError:
            print 'Reference CDT XML ' + ref_cdt_xml + ' not found. Missed --skip_export option?'
            return -1

    if skipexport:
        try:
            with open(cdt_mod, 'r'):
                print 'cdt_mod.py location: ' + cdt_mod
        except IOError:
            print 'cdt_mod.py not found. Try giving correct path like --cdt_mod=<path>'
            return -1

    try:
        with open(smem_gen, 'r'):
            print 'smem-tool.py location: ' + smem_gen
    except IOError:
        print 'smem-tool.py not found. Incomplete source?'
        return -1

    try:
        with open(bootconfig_gen, 'r'):
            print 'bootconfig_tool location: ' + bootconfig_gen
    except IOError:
        print 'bootconfig_tool not found. Incomplete source?'
        return -1

    return 0


def process_nand(Config, section, brdoutpath):
    global mbn_gen
    global configdir

    nand_pagesize = Config.getint(section, 'nand_pagesize')
    nand_pages_per_block = Config.getint(section, 'nand_pages_per_block')
    nand_total_blocks = Config.getint(section, 'nand_total_blocks')
    nand_partition = Config.get(section, 'nand_partition')
    nand_partition = os.path.join(configdir, nand_partition)

    nandsyspartition = Config.get(section, 'nand_partition_mbn')

    print '\tNand page size: ' + str(nand_pagesize) + ', pages/block: ' \
        + str(nand_pages_per_block) + ', total blocks: ' \
        + str(nand_total_blocks)
    print '\tPartition info: ' + nand_partition

      # Create user partition

    print '\tCreating user partition',
    prc = subprocess.Popen(['python', mbn_gen, nand_partition,
                           nanduserpartition], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create user partition'
        return prc.returncode
    else:
        print '...User partition created'

    userpart_path = os.path.join(brdoutpath, nanduserpartition)

      # Create nand partition

    print '\tCreating system partition',
    prc = subprocess.Popen([
        nor_tool,
        '-s',
        str(nand_pagesize),
        '-p',
        str(nand_pages_per_block),
        '-b',
        str(nand_total_blocks),
        '-u',
        userpart_path,
        '-o',
        nandsyspartition,
        ], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create system partition'
        return prc.returncode
    else:
        print '...System partition created'

    return 0

def process_emmc(Config, section, brdoutpath):
    global ptool
    global msp
    global configdir

    emmc_total_blocks = Config.getint(section, 'emmc_total_blocks')
    emmc_partition = Config.get(section, 'emmc_partition')
    emmc_partition = os.path.join(configdir, emmc_partition)

    emmcsyspartition = Config.get(section, 'emmc_partition_mbn')

    print '\tTotal blocks: ' + str(emmc_total_blocks)
    print '\tPartition info: ' + emmc_partition

      # Create user partition

    print '\tCreating rawprogram0.xml and patch0.xml',
    prc = subprocess.Popen(['python', ptool, '-x',
                           emmc_partition], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create rawprogram0.xml and patch0.xml'
        return prc.returncode
    else:
        print '...rawprogram0.xml and patch0.xml created'

    rawprogram_path = os.path.join(brdoutpath, 'rawprogram0.xml')
    patch_path = os.path.join(brdoutpath, 'patch0.xml')

    print '\t rawprogram' + rawprogram_path
    print '\t patch' + patch_path

      # Create nand partition

    print '\tRunning msp.py to update gpt_main0.bin partition',
    prc = subprocess.Popen([
        'python',
        msp,
        '-r',
        rawprogram_path,
        '-p',
        patch_path,
        '-d',
        str(emmc_total_blocks),
        '-n',
        ], cwd=brdoutpath)
    prc.wait()

    print '\tCreating rawprogram1.xml and patch1.xml',
    prc = subprocess.Popen(['python', ptool, '-x',
                           emmc_partition, '-p', '1'], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create rawprogram1.xml and patch1.xml'
        return prc.returncode
    else:
        print '...rawprogram0.xml and patch1.xml created'

    rawprogram_path = os.path.join(brdoutpath, 'rawprogram1.xml')
    patch_path = os.path.join(brdoutpath, 'patch1.xml')

    print '\t rawprogram' + rawprogram_path
    print '\t patch' + patch_path

      # Create nand partition

    print '\tRunning msp.py to update gpt_main0.bin partition',
    prc = subprocess.Popen([
        'python',
        msp,
        '-r',
        rawprogram_path,
        '-p',
        patch_path,
        '-d',
        str(emmc_total_blocks),
        '-n',
        ], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create system partition'
        return prc.returncode
    else:
        print '...System partition created'

    return 0


def process_nor_common(Config, section, brdoutpath, xmlentry, userpart, syspart):
    global mbn_gen
    global configdir

    nor_pagesize = Config.getint(section, 'nor_pagesize')
    nor_pages_per_block = Config.getint(section, 'nor_pages_per_block')
    nor_total_blocks = Config.getint(section, 'nor_total_blocks')
    nor_partition = os.path.join(configdir, xmlentry)

    print '\tNor page size: ' + str(nor_pagesize) + ', pages/block: ' \
        + str(nor_pages_per_block) + ', total blocks: ' \
        + str(nor_total_blocks) + ', partition info: ' + nor_partition

      # Create user partition

    print '\tCreating user partition',
    prc = subprocess.Popen(['python', mbn_gen, nor_partition,
                            userpart], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create user partition'
        return prc.returncode
    else:
        print '...User partition created'

    userpart_path = os.path.join(brdoutpath, userpart)

      # Create nand partition

    print '\tCreating system partition',
    prc = subprocess.Popen([
        nor_tool,
        '-s',
        str(nor_pagesize),
        '-p',
        str(nor_pages_per_block),
        '-b',
        str(nor_total_blocks),
        '-c',
        str(1),
        '-u',
        userpart_path,
        '-o',
        syspart,
        ], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create system partition'
        return prc.returncode
    else:
        print '...System partition created'

    return 0


def process_nor(Config, section, brdoutpath):
    return process_nor_common(Config, section, brdoutpath, Config.get(section, 'nor_partition'), noruserpartition,
                              Config.get(section, 'nor_partition_mbn'))

def process_nor_v2(Config, section, brdoutpath):
    return process_nor_common(Config, section, brdoutpath, Config.get(section, 'nor_partition_v2'), noruserpartition_v2,
                              Config.get(section, 'nor_partition_mbn_v2'))

def process_nor_plus_nand(Config, section, brdoutpath):
    return process_nor_common(Config, section, brdoutpath, Config.get(section, 'norplusnand_partition'),
                              norplusnandusrpartition, Config.get(section, 'norplusnand_partition_mbn'))

def process_nor_plus_emmc(Config, section, brdoutpath):
    return process_nor_common(Config, section, brdoutpath, Config.get(section, 'norplusemmc_partition'),
                              norplusemmcusrpartition, Config.get(section, 'norplusemmc_partition_mbn'))

def process_smem(smem_info, brdoutpath):
    global configdir

    smem_info_file = os.path.join(configdir, smem_info)
    try:
        with open(smem_info_file, 'r'):
            print '\tsmem_info_file location: ' + smem_info_file
    except IOError:
        print 'smem_info_file not found. Have you placed it in (' \
            + smem_info_file + ')?'
        return -1

    print '\tCreating smem binary',
    new_smeminfobin = os.path.splitext(smem_info)[0] + os.path.extsep + "bin"
    prc = subprocess.Popen(['python', smem_gen, smem_info_file,
                           smeminfobin], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create smem binary'
        return prc.returncode
    else:
        print '...smem binary created'

    shutil.copyfile(os.path.join(brdoutpath, smeminfobin),
            os.path.join(brdoutpath, new_smeminfobin))
    return 0


def process_cdtinfo(cdt_info, brdoutpath):
    cdt_info_file = os.path.join(configdir, cdt_info)
    try:
        with open(cdt_info_file, 'r'):
            print '\tcdt_info_file location: ' + cdt_info_file
    except IOError:
        print 'cdt_info_file not found. Have you placed it in (' \
            + cdt_info_file + ')?'
        return -1

    print '\tCreating CDT binary',
    new_cdtbin = os.path.splitext(cdt_info)[0] + os.path.extsep + "mbn"
    prc = subprocess.Popen(['python', cdt_gen, cdt_info_file, cdtbin],
                           cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create CDT binary'
        return prc.returncode
    else:
        print '...CDT binary created'

    shutil.copyfile(os.path.join(brdoutpath, cdtbin),
            os.path.join(brdoutpath, new_cdtbin))
    return 0


def process_cdtmod(args_raw, brdoutpath):
    print '\tApplying CDT mod',

    outpath = os.path.abspath(args_raw.cdt_bin)
    if args_raw.cdt_outbin is not None:
        outpath = os.path.abspath(args_raw.cdt_outbin)

    prc = subprocess.Popen(['python', os.path.abspath(args_raw.cdt_mod), '--cdt_bin='+os.path.abspath(args_raw.cdt_bin),
                            '--cdt_modxml='+os.path.abspath(args_raw.cdt_modxml),
                            '--cdt_outbin='+outpath], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to mod CDT binary'
        return prc.returncode
    else:
        print '...CDT mod applied'

    shutil.copy2(outpath, brdoutpath)
    print '\tModified cb-cdt.mbn copied to ' + str(brdoutpath)
    return 0

def process_ssdinfo(ssd_info, brdoutpath):
    ssdbin_file = os.path.join(brdoutpath, ssdbin)
    if ssd_info.lower() == 'none':
        print '\tCreating SSD Binary ',
        try:
            with open(ssdbin_file, 'wb') as f:
                for x in range(8 * 1024):
                    f.write("\0")
                f.close()
                print '...SSD Binary created'
        except IOError:
            print 'Unable to write file: ' + ssdbin
            raise
    return 0

def process_nand_bootconfig(Config, section, brdoutpath):
    global mbn_gen
    global configdir

    nand_pagesize = Config.getint(section, 'nand_pagesize')
    nand_pages_per_block = Config.getint(section, 'nand_pages_per_block')
    bootconfig = Config.get(section, 'bootconfig')
    bootconfig = os.path.join(configdir, bootconfig)

    nand_bootconfig = Config.get(section, 'nand_bootconfig')

    print '\tNand page size: ' + str(nand_pagesize) + ', pages/block: ' \
        + str(nand_pages_per_block)
    print '\tBootconfig info: ' + bootconfig

      # Create user partition

    print '\tCreating raw partition',
    prc = subprocess.Popen(['python', mbn_gen, bootconfig,
                           nand_raw_bootconfig], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create raw partition'
        return prc.returncode
    else:
        print '...Raw partition created'

    rawpart_path = os.path.join(brdoutpath, nand_raw_bootconfig)

      # Create nand partition

    print '\tCreating Nand bootconfig partition',
    prc = subprocess.Popen([
        bootconfig_gen,
        '-s',
        str(nand_pagesize),
        '-p',
        str(nand_pages_per_block),
        '-i',
        rawpart_path,
        '-o',
        nand_bootconfig,
        ], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create system partition'
        return prc.returncode
    else:
        print '...System partition created'

    return 0

def process_nand_bootconfig1(Config, section, brdoutpath):
    global mbn_gen
    global configdir

    nand_pagesize = Config.getint(section, 'nand_pagesize')
    nand_pages_per_block = Config.getint(section, 'nand_pages_per_block')
    bootconfig1 = Config.get(section, 'bootconfig1')
    bootconfig1 = os.path.join(configdir, bootconfig1)

    nand_bootconfig1 = Config.get(section, 'nand_bootconfig1')

    print '\tNand page size: ' + str(nand_pagesize) + ', pages/block: ' \
        + str(nand_pages_per_block)
    print '\tBootconfig info: ' + bootconfig1

      # Create user partition

    print '\tCreating raw partition',
    prc = subprocess.Popen(['python', mbn_gen, bootconfig1,
                           nand_raw_bootconfig1], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create raw partition'
        return prc.returncode
    else:
        print '...Raw partition created'

    rawpart_path = os.path.join(brdoutpath, nand_raw_bootconfig1)

      # Create nand partition

    print '\tCreating Nand bootconfig partition',
    prc = subprocess.Popen([
        bootconfig_gen,
        '-s',
        str(nand_pagesize),
        '-p',
        str(nand_pages_per_block),
        '-i',
        rawpart_path,
        '-o',
        nand_bootconfig1,
        ], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create system partition'
        return prc.returncode
    else:
        print '...System partition created'

    return 0

def process_nor_bootconfig1(Config, section, brdoutpath):
    global mbn_gen
    global configdir

    nor_pagesize = Config.getint(section, 'nor_pagesize')
    nor_pages_per_block = Config.getint(section, 'nor_pages_per_block')
    bootconfig1 = Config.get(section, 'bootconfig1')
    bootconfig1 = os.path.join(configdir, bootconfig1)

    nor_bootconfig1 = Config.get(section, 'nor_bootconfig1')

    print '\tNor page size: ' + str(nor_pagesize) + ', pages/block: ' \
        + str(nor_pages_per_block)
    print '\tBootconfig info: ' + bootconfig1

      # Create user partition

    print '\tCreating raw partition',
    prc = subprocess.Popen(['python', mbn_gen, bootconfig1,
                           nor_raw_bootconfig1], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create raw partition'
        return prc.returncode
    else:
        print '...Raw partition created'

    rawpart_path = os.path.join(brdoutpath, nor_raw_bootconfig1)

      # Create nand partition

    print '\tCreating Nand bootconfig partition',
    prc = subprocess.Popen([
        bootconfig_gen,
        '-s',
        str(nor_pagesize),
        '-p',
        str(nor_pages_per_block),
        '-i',
        rawpart_path,
        '-o',
        nor_bootconfig1,
        ], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create system partition'
        return prc.returncode
    else:
        print '...System partition created'

    return 0


def process_nor_bootconfig(Config, section, brdoutpath):
    global mbn_gen
    global configdir

    nor_pagesize = Config.getint(section, 'nor_pagesize')
    nor_pages_per_block = Config.getint(section, 'nor_pages_per_block')
    bootconfig = Config.get(section, 'bootconfig')
    bootconfig = os.path.join(configdir, bootconfig)

    nor_bootconfig = Config.get(section, 'nor_bootconfig')

    print '\tNor page size: ' + str(nor_pagesize) + ', pages/block: ' \
        + str(nor_pages_per_block)
    print '\tBootconfig info: ' + bootconfig

      # Create user partition

    print '\tCreating raw partition',
    prc = subprocess.Popen(['python', mbn_gen, bootconfig,
                           nor_raw_bootconfig], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create raw partition'
        return prc.returncode
    else:
        print '...Raw partition created'

    rawpart_path = os.path.join(brdoutpath, nor_raw_bootconfig)

      # Create nand partition

    print '\tCreating Nand bootconfig partition',
    prc = subprocess.Popen([
        bootconfig_gen,
        '-s',
        str(nor_pagesize),
        '-p',
        str(nor_pages_per_block),
        '-i',
        rawpart_path,
        '-o',
        nor_bootconfig,
        ], cwd=brdoutpath)
    prc.wait()
    if prc.returncode != 0:
        print 'ERROR: unable to create system partition'
        return prc.returncode
    else:
        print '...System partition created'

    return 0



def main():
    global sbl_path
    global nor_tool
    global mbn_gen
    global cdt_gen
    global smem_gen
    global configdir
    global cdt_mod
    global cb_configs
    global ref_cdt_xml
    global cbreadme
    global bootconfig_gen
    global ptool
    global msp

    parser = \
        argparse.ArgumentParser(description='Creates misc. images required for programming the board'
                                )
    parser.add_argument('-c', '--configdir',
                        help='Configuration directory', default='config'
                        )
    parser.add_argument('-o', '--outdir', help='Output directory',
                        default='out')
    parser.add_argument('-r', '--sblrootdir', help='SBL root directory'
                        , default='../..')
    parser.add_argument('-s', '--skip_export', help="Skips exporting tools", action="store_true")
    parser.add_argument("-n", "--nor_tool", help="nor_tool with path")
    parser.add_argument("-m", "--mbn_gen", help="nand_mbn_generator.py with path")
    parser.add_argument("-e", "--smem_gen", help="smem-tool.py with path")
    parser.add_argument("-b", "--bootconfig_gen", help="bootconfig_tool with path")

    parser.add_argument("-x", "--cdt_mod", help="cdt_mod.py with path can only be used with --skip_export option")
    parser.add_argument("-y", "--cdt_bin", help="cdt.mbn with path")
    parser.add_argument("-t", "--cdt_outbin", help="name for output cdt.mbn")
    parser.add_argument("-z", "--cdt_modxml", help="cdt_mod.xml with path")
    parser.add_argument("-p", "--ptool", help="ptool.py with path")
    parser.add_argument("-a", "--msp", help="msp.py with path")

    args_raw = parser.parse_args()
    args = vars(parser.parse_args())

    path = os.getcwd()

    outputdir = os.path.join(path, args['outdir'])
    configdir = os.path.join(path, args['configdir'])
    sbl_path_rel = os.path.join(path, os.path.relpath(args['sblrootdir'
                                ]))

    sbl_path = os.path.abspath(sbl_path_rel)
    configfilename = os.path.join(configdir, configfile)

    print 'SBL dir: ' + sbl_path
    print 'Config dir: ' + configdir
    print 'Output dir:' + outputdir

    if args_raw.nor_tool is not None:
        nor_tool = os.path.abspath(args_raw.nor_tool)
        os.chmod(nor_tool, 0744)
    else:
        nor_tool = nor_tool.replace('$$', sbl_path)

    if args_raw.mbn_gen is not None:
        mbn_gen = os.path.abspath(args_raw.mbn_gen)
    else:
        mbn_gen = mbn_gen.replace('$$', sbl_path)

    if args_raw.cdt_mod is not None:
        cdt_mod = os.path.abspath(args_raw.cdt_mod)
    else:
        cdt_mod = cdt_mod.replace('$$', sbl_path)

    if args_raw.ptool is not None:
        ptool = os.path.abspath(args_raw.ptool)
    else:
        ptool = ptool.replace('$$', sbl_path)

    if args_raw.msp is not None:
        msp = os.path.abspath(args_raw.msp)
    else:
        msp = msp.replace('$$', sbl_path)

    cdt_gen = cdt_gen.replace('$$', sbl_path)
    cb_configs = cb_configs.replace('$$', sbl_path)
    ref_cdt_xml = ref_cdt_xml.replace('$$', sbl_path)
    cbreadme = cbreadme.replace('$$', sbl_path)

    if args_raw.smem_gen is not None:
        smem_gen = os.path.abspath(args_raw.smem_gen)
    else:
        smem_gen = smem_gen.replace('$$', sbl_path)

    if args_raw.bootconfig_gen is not None:
        bootconfig_gen = os.path.abspath(args_raw.bootconfig_gen)
    else:
        bootconfig_gen = bootconfig_gen.replace('$$', sbl_path)

    os.chmod(bootconfig_gen, 0744)

    if check_path(args_raw.skip_export) < 0:
        return -1

    try:
        with open(configfilename):
            Config = ConfigParser.SafeConfigParser({'smem_info': '',
                    'cdt_info': '', 'ssd_info': 'none'})
            Config.read(configfilename)
            configlist = Config.sections()

            for section in configlist:
                boardir = Config.get(section, 'dirname')
                print ''
                print 'Processing ' + section
                brdoutpath = os.path.abspath(os.path.join(outputdir,
                        boardir))
                try:  # creating output directory
                    os.makedirs(brdoutpath)
                    print '\tCreated board directory: ' + brdoutpath
                except OSError, exc:
                    if exc.errno == errno.EEXIST \
                        and os.path.isdir(path):
                        print '\tUsing existing directory: ' \
                            + brdoutpath
                    else:
                        raise

                print ''

                  # Process NAND

                nandavailable = Config.getboolean(section,
                        'nand_available')
                if nandavailable:
                    if process_nand(Config, section, brdoutpath) < 0:
                        return -1
                else:
                    print '\tNo Nand flash available'

                print ''

                  # Process NOR

                noravailable = Config.getboolean(section,
                        'nor_available')
                if noravailable:
                    if process_nor(Config, section, brdoutpath) < 0:
                        return -1
                else:
                    print '\tNo Nor flash available'

                  # Process NOR v2

                noravailable_v2 = Config.getboolean(section,
                        'nor_available_v2')
                if noravailable_v2:
                    if process_nor_v2(Config, section, brdoutpath) < 0:
                        return -1
                else:
                    print '\tNo Nor flash available'

                print ''

                emmcavailable = Config.getboolean(section,
                        'emmc_available')
                if emmcavailable:
                    if process_emmc(Config, section, brdoutpath) < 0:
                        return -1
                else:
                    print '\tNo emmc flash available'

                print ''

                # Process NOR_NAND
                norandnandavailable = Config.getboolean(section, 'norplusnand_available')

                if norandnandavailable:
                    if process_nor_plus_nand(Config, section, brdoutpath) < 0:
                        return -1
                else:
                    print  '\tNo NOR+NAND combination!'

                # Process NOR_EMMC
                norandemmcavailable = Config.getboolean(section, 'norplusemmc_available')

                if norandemmcavailable:
                    if process_nor_plus_emmc(Config, section, brdoutpath) < 0:
                        return -1
                else:
                    print  '\tNo NOR+EMMC combination!'

                # Process smem_info
                smem_info = Config.get(section, 'smem_info')
                if len(smem_info) > 0:
                    if process_smem(smem_info, brdoutpath) < 0:
                        return -1
                else:
                    print 'No smem_info given skipping'

                print ''

                if args_raw.skip_export is False:
                    # Process cdt_info
                    cdt_info = Config.get(section, 'cdt_info')
                    if len(cdt_info) > 0:
                        if process_cdtinfo(cdt_info, brdoutpath) < 0:
                            return -1
                    else:
                        print 'No cdt_info given skipping'

                print ''

                if args_raw.cdt_mod is not None and args_raw.skip_export is True:
                    if process_cdtmod(args_raw, brdoutpath) < 0:
                        return -1
                    pass
                print ''

                  # Process ssd_info

                ssd_info = Config.get(section, 'ssd_info')

                  # prc = subprocess.Popen(["python", ssd_gen, ssd_info_file, ssdbin], cwd=brdoutpath)

                if process_ssdinfo(ssd_info, brdoutpath) < 0:
                    print "Not implemetend, ssd_info currently supports only 'none'"
                    return -1
                else:
                    pass

		# Process bootconfig_info

                if process_nand_bootconfig(Config, section, brdoutpath) < 0:
                    return -1
                else:
                    pass

                if process_nand_bootconfig1(Config, section, brdoutpath) < 0:
                    return -1
                else:
                    pass

                if process_nor_bootconfig(Config, section, brdoutpath) < 0:
                    return -1
                else:
                    pass

                if process_nor_bootconfig1(Config, section, brdoutpath) < 0:
                    return -1
                else:
                    pass

        print
        print "Processing boardconfig"
        shutil.copyfile(os.path.join(configdir, "boardconfig"),
                os.path.join(outputdir, "boardconfig"))
        print "\tboardconfig copied to " + outputdir

        print
        print "Processing boardconfig_v2"
        shutil.copyfile(os.path.join(configdir, "boardconfig_v2"),
                os.path.join(outputdir, "boardconfig_v2"))
        print "\tboardconfig_v2 copied to " + outputdir

	print
        print "Processing appsboardconfig"
        shutil.copyfile(os.path.join(configdir, "appsboardconfig"),
                os.path.join(outputdir, "appsboardconfig"))
        print "\tappsboardconfig copied to " + outputdir

        if args_raw.skip_export is True:
            print ""
        else:
            print "\n\tTools are exported. Use --skip_export option to overcome this operation"
            toolsoutpath = os.path.abspath(os.path.join(outputdir, "tools"))
            try:  # creating output directory
                os.makedirs(toolsoutpath)
                print '\tCreated tools directory: ' + toolsoutpath
            except OSError, exc:
                if exc.errno == errno.EEXIST and os.path.isdir(path):
                    print '\tUsing existing directory: ' + toolsoutpath
                else:
                    raise
            shutil.copyfile(nor_tool, os.path.join(toolsoutpath, "nor_tool"))
            shutil.copyfile(mbn_gen, os.path.join(toolsoutpath, "nand_mbn_generator.py"))
            shutil.copyfile(cdt_mod, os.path.join(toolsoutpath, "cdt_mod.py"))
            shutil.copyfile(smem_gen, os.path.join(toolsoutpath, "smem-tool.py"))
            shutil.copyfile(bootconfig_gen, os.path.join(toolsoutpath, "bootconfig_tool"))
            shutil.copyfile(ptool, os.path.join(toolsoutpath, "ptool.py"))
            shutil.copyfile(msp, os.path.join(toolsoutpath, "msp.py"))
            shutil.copyfile(cbreadme, os.path.join(toolsoutpath, "README.txt"))
            shutil.copyfile(os.path.abspath(sys.argv[0]), os.path.join(toolsoutpath, "genimg.py"))
            configcopypath = os.path.join(toolsoutpath, "config")
            shutil.rmtree(configcopypath, ignore_errors=True)
            shutil.copytree(cb_configs, configcopypath)

            print '\tCreating CD CDT binary',
            prc = subprocess.Popen(['python', cdt_gen, ref_cdt_xml, 'cb-cdt.mbn'], cwd=toolsoutpath)
            prc.wait()
            if prc.returncode != 0:
                print 'ERROR: unable to create reference CB CDT binary'
                return prc.returncode
            else:
                os.remove(os.path.join(toolsoutpath, "boot_cdt_array.c"))
        print '''

Images are in :''' + outputdir + '''

'''
    except IOError:
        print configfilename + ' not found! Cannot procced.'
        print """ Create a config file like this:
[DB149]                       # Board Name
dirname=DB149                 # Output directory name
nand_available=true           # value can be true or false, if false reset of the nand_* params are not required
nor_available=true            # value can be true or false, if false reset of the nor_* params are not required
nand_pagesize=2048            # Page size in bytes as integer value
nand_pages_per_block=64       # Number of pages per block as integer value
nand_total_blocks=1024        # Total number of blocks present in the flash
nand_partition=nand-partition.xml  # parition file name which should be present along with this file
nor_pagesize=256              # Page size in bytes as integer value
nor_pages_per_block=1024      # Number of pages per block as integer value
nor_total_blocks=256          # Total number of blocks present in the flash
nor_partition=nor-partition.xml  # parition file name which should be present along with this file
smem_info=smem_min.xml        # smem information file, this line entry not required when smem bin not used
cdt_info=DB149_cdt.xml        # DDR, platformid and machine id configuration should present in the specified file
ssd_info=none                 # ssd information file, this line entry not required when ssd bin is not created
norplusnand_partition=nor_and_nand_parition.xml # partition file name for nor boot and nand fs combination, no partition for nand as of now
norplusnand_available=true    # this board has nor boot and nand fs option
norplusnand_flash_conf=nor_and_nand_flash.conf # input for pack.py
                      """


if __name__ == '__main__':
    main()
