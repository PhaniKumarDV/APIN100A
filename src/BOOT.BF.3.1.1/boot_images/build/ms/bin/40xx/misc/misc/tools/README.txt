Custom Board Configuration
--------------------------

[1] Image creation:
	To create the images using the configuration given in config/ directory
  use the command: "python genimg.py --partition_tool=partition_tool --mbn_gen=nand_mbn_generator.py
  --cdt_mod=cdt_mod.py --smem_gen=smem-tool.py --configdir=config/ --skip_export
  --cdt_bin=cb-cdt.mbn --cdt_modxml=config/cb-cdtmod.xml --cdt_outbin=cb-cdtnew.mbn --image_name=NAND_IMAGES"
  provided the partition_tool, nand_mbn_generator.py, cdt_mod.py, smem_tool.py, cb-cdt.mbn
  and the config directory are present in the current directory. Replace image name "NAND_IMAGES" with 
  the required image name to be generated. List of images available are NAND_IMAGES, NOR_IMAGES, EMMC_IMAGES,
  NOR_PLUS_NAND_IMAGES, SMEM_IMAGES, CDT_IMAGES. If the listed
  tools are moved then give the corresponding path (e.g --partition_tool=/tmp/partition_tool)
  in the arguments.
	The output is created in the directory named out/ by default, this can be
  modified by adding one more arugment --outdir (e.g --outdir=/my/path/out).

 NOTE: The scripts expect the file boardconfig, so renaming it will not be possible
       without chaning scripts. However other files can be renamed (though not
       recommended) and in that case it should be refelected in the boardconfig
       file


[2] Parition table customization:
	To customize patition table the XML for corresponding flash needs to be
  edited. For example for NAND boot edit the file config/nand-partition.xml and
  run the command as given in [1]

	[2.0] To add a new entry copy an existing parition entry and modify
	      according to the below rules

	[2.1] In the partition XML the size can be given as size_kb or size_block
	      options. If size_kb is given then last attribute should be 0xFF,
	      whereus the size_block should have the last attribute as 0xFE

	[2.2] If the size_kb option is given as 0xFFFFFFFF, it is taken as grow
	      parition, i.e all the remaining space is used for that pariticular
	      parition. This makes only the last entry should be given grow
	      parition size

	[2.3] The parition 0:SBL1 cannot be reordered and it always should be the
	      first entry


[3] DDR Parameter customization:
	To customize DDR parameters edit the XML file config/cb-cdtmod.xml file
  and run the command given in [1]
	The customization of DDR is possible with the values given in the
  config/cb-cdtmod.xml. These values are taken and patched in existing cdt.mbn
  binary. The number of rows, column and bank should match with the DDR being
  used and can be obtained from the DDR data sheet. If there is mismatch in these
  values it would create boot failure. Only DDR3 is supported using given XML file.

[4] Machine ID customization:
	It is important to *change* the machine ID as all the boot and initialization
  process depends on the machine ID. Once necessary changes done in the application
  bootloader and HLOS for the machine ID, the machine ID needs to be entered in the
  below files:
 	* config/boardconfig
	* config/cb-cdtmod.xml

