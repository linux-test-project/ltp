#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		                                      ##
##									      ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	pciutils.sh
#
# Description:	Test pciutils package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	Sept 09 2003 - Created AP
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
################################################################################
# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=19
REQUIRED="fax2ps fax2tiff gif2tiff pal2rgb ppm2tiff ras2tiff rgb2ycbcr \
	thumbnail tiff2bw tiff2rgba tiffcmp tiffcp tiffdither tiffdump \
	tiffinfo tiffmedian tiffset tiffsplit grep"

COMMANDS="tiffinfo gif2tiff tiffdump tiffcmp tiffcp tiffsplit tiff2bw \
	tiff2rgba tiffmedian rgb2ycbcr fax2tiff fax2ps pal2rgb ppm2tiff \
	ras2tiff thumbnail tiffset"
################################################################################
function Notest()
{
	tc_info "$1 Not tested"
}	
################################################################################
# testcase functions
################################################################################
function TC_gif2tiff()
{
	tc_register "gif2tiff"
	gif2tiff $LTPBIN/tiff/test.gif $TCTMP/t.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	tiffinfo $TCTMP/t.tif >$stdout 2>$stderr
	grep -q 320 $stdout &&
	grep -q "single image plane" $stdout &&
	grep -q PackBits $stdout 
	tc_pass_or_fail $? "Unexpected output."
}

function TC_tiffinfo()            
{
	tc_register "tiffinfo"
	tiffinfo $LTPBIN/tiff/x.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	grep -q 320 $stdout &&
	grep -q "single image plane" $stdout &&
	grep -q PackBits $stdout 
	tc_pass_or_fail $? "Unexpected output." || return
	
	tc_register "tiffinfo -c"
	let TST_TOTAL+=1
	tiffinfo -c $LTPBIN/tiff/x.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	grep -q "Color Map" $stdout &&
	grep -q "10663 10663" $stdout &&
	grep -q PackBits $stdout 
	tc_pass_or_fail $? "Unexpected output."
	
	tc_register "tiffinfo -s"
	let TST_TOTAL+=1
	tiffinfo -s $LTPBIN/tiff/x.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	grep -q "8 Strips" $stdout &&
	grep -q 9639 $stdout &&
	grep -q PackBits $stdout 
	tc_pass_or_fail $? "Unexpected output."
}

function TC_tiffdump()            
{
	tc_register "tiffdump"
	tiffdump  $LTPBIN/tiff/x.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return


	grep -q "<little-endian>" $stdout &&
	grep -q "Orientation (274) SHORT (3) 1<1>" $stdout &&
	grep -q Colormap $stdout 
	tc_pass_or_fail $? "Unexpected output." || return

	tc_register "tiffdump -h"
	let TST_TOTAL+=1
	tiffdump  -h $LTPBIN/tiff/x.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	grep -q 0x $stdout 
	tc_pass_or_fail $? "Unexpected output."
}
function TC_fax2ps()            
{
	tc_register "fax2ps"
	fax2ps $LTPBIN/tiff/fax.tif  >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	grep -q fax2ps $stdout 
	tc_pass_or_fail $? "Unexpected output."
}
function TC_fax2tiff()            
{
	tc_register "fax2tiff -1 -p -M -R"
	fax2tiff -o $TCTMP/f.tif -1 -p -M -R 98 $LTPBIN/tiff/fax.g3 \
		>$stdout 2>/dev/null
	tc_fail_if_bad $? "Incorrect return code" || return

	tiffinfo $TCTMP/f.tif > $stdout
	grep -q CCITT $stdout &&
	grep -q fax $stdout
	tc_pass_or_fail $? "Unexpected output."
}

function TC_pal2rgb()            
{
	tc_register "pal2rgb"
	pal2rgb $LTPBIN/tiff/pal.tif $TCTMP/p.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	tiffinfo $TCTMP/p.tif | grep -q Deflate 
	tc_pass_or_fail $? "Unexpected output."

	tc_register "pal2rgb -pr"
	let TST_TOTAL+=1
	pal2rgb -r 8 -p contig $LTPBIN/tiff/pal.tif  $TCTMP/p2.tif >$stdout \
		2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	tiffinfo $TCTMP/p2.tif | grep "Rows/Strip" | grep -q 8 
	tc_pass_or_fail $? "Unexpected output."
}

function TC_ppm2tiff()            
{
	Notest ppm2tiff
}
function TC_ras2tiff()            
{
	Notest ras2tiff
}

function TC_rgb2ycbcr()            
{
	tc_register "rgb2ycbcr"
	rgb2ycbcr $LTPBIN/tiff/mcp.tif $TCTMP/ycb.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	tiffinfo $TCTMP/ycb.tif >$stdout 2>$stderr
	grep -q YCbCr $stdout &&
	grep -q 0.299 $stdout &&
	grep -q 255 $stdout 
	tc_pass_or_fail $? "Unexpected output."

	tc_register "rgb2ycbcr -cr"
	let TST_TOTAL+=1
	rgb2ycbcr -c none -r 8 $LTPBIN/tiff/mcp.tif $TCTMP/ycb2.tif \
		>$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return
	
	tiffinfo $TCTMP/ycb2.tif >$stdout
	grep -q 8 $stdout &&
	grep -q None $stdout
	tc_pass_or_fail $? "Unexpected output."
}


function TC_thumbnail()            
{
	Notest thumbnail
}


function TC_tiff2bw()            
{
	tc_register "tiff2bw"
	tiff2bw $LTPBIN/tiff/mcp.tif $TCTMP/bw.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	tiffinfo $TCTMP/bw.tif >$stdout 2>$stderr
	grep -q "mcp.tif" $stdout &&
	grep -q "min-is-black" $stdout &&
	grep -q "Compression Scheme: None" $stdout
	tc_pass_or_fail $? "Unexpected output." || return
	
	tc_register "tiff2bw -c"
	let TST_TOTAL+=1
	tiff2bw -c zip $LTPBIN/tiff/mcp.tif $TCTMP/bw2.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	tiffinfo $TCTMP/bw2.tif >$stdout 2>$stderr
	grep -q "mcp.tif" $stdout &&
	grep -q "min-is-black" $stdout &&
	grep -q "Deflate" $stdout
	tc_pass_or_fail $? "Unexpected output."
}


function TC_tiff2ps()            
{
	tc_register "tiff"
	tiff >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	grep -q $stdout 
	tc_pass_or_fail $? "Unexpected output."
}

function TC_tiff2rgba()            
{
	tc_register "tiff2rgba"
	tiff2rgba $LTPBIN/tiff/mcp.tif $TCTMP/rgba.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	tiffinfo $TCTMP/rgba.tif >$stdout 2>$stderr
	grep -q "PackBits" $stdout && 
	grep -q "1<assoc-alpha>" $stdout
	tc_pass_or_fail $? "Unexpected output." || return
	
	tc_register "tiff2rgba -nc"
	let TC_TOTAL+=1
	tiff2rgba -n -c zip $LTPBIN/tiff/mcp.tif $TCTMP/rgba.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	tiffinfo $TCTMP/rgba.tif >$stdout 2>$stderr
	grep -q "1<assoc-alpha>" $stdout
	[ $? -ne 0 ] &&
	grep -q "Deflate" $stdout  
	tc_pass_or_fail $? "Unexpected output."
	
}


function TC_tiffcmp()            
{
	tc_register "tiffcmp"
	tiffcmp $LTPBIN/tiff/x.tif $LTPBIN/tiff/x.tif >$stdout 2>$stderr
	tc_pass_or_fail $? "Not function correctly." || return

	tc_register "tiffcmp2"
	let TC_TOTAL+=1
	tiffcmp $LTPBIN/tiff/x.tif $LTPBIN/tiff/y.tif >$stdout 2>$stderr
	[ $? -ne 0 ]
	tc_pass_or_fail $? "Not function correctly." || return
}


function TC_tiffcp()            
{
	tc_register "tiffcp"
	tiffcp $LTPBIN/tiff/x.tif $LTPBIN/tiff/x2.tif $TCTMP/z.tif \
		>$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code." || return

	tiffinfo $TCTMP/z.tif > $TCTMP/tiffcp.res 2>/dev/null
	grep -q PackBits $TCTMP/tiffcp.res &&
	grep -q Deflate $TCTMP/tiffcp.res &&
	grep -q "single image plane" $TCTMP/tiffcp.res 
	tc_pass_or_fail $? "Unexpected output." || return

	tc_register "tiffcp -p"
	let TC_TOTAL+=1
	tiffcp -p separate $LTPBIN/tiff/x.tif $LTPBIN/tiff/x2.tif $TCTMP/z.tif \
		>$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code." || return

	tiffinfo $TCTMP/z.tif > $TCTMP/tiffcp2.res 2>/dev/null
	grep -q PackBits $TCTMP/tiffcp2.res &&
	grep -q Deflate $TCTMP/tiffcp2.res &&
	grep -q "separate image plane" $TCTMP/tiffcp2.res 
	tc_pass_or_fail $? "Unexpected output."

	tc_register "tiffcp -r"
	let TC_TOTAL+=1
	tiffcp -r 20 $LTPBIN/tiff/x.tif $LTPBIN/tiff/x2.tif $TCTMP/z.tif \
		>$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code." || return

	tiffinfo $TCTMP/z.tif > $TCTMP/tiffcp.res3 2>/dev/null
	grep -q PackBits $TCTMP/tiffcp.res3 &&
	grep -q Deflate $TCTMP/tiffcp.res3 &&
	grep -q "Rows/Strip: 20" $TCTMP/tiffcp.res3 
	tc_pass_or_fail $? "Unexpected output."

	tc_register "tiffcp -t"
	let TC_TOTAL+=1
	tiffcp -t $LTPBIN/tiff/x.tif $LTPBIN/tiff/x2.tif $TCTMP/z.tif \
		>$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code." || return

	tiffinfo $TCTMP/z.tif > $TCTMP/tiffcp.res3 2>/dev/null
	grep -q "Rows/Strip: 25" $TCTMP/tiffcp.res3 
	[ $? -ne 0 ] &&
	grep -q PackBits $TCTMP/tiffcp.res3 &&
	grep -q Deflate $TCTMP/tiffcp.res3 
	tc_pass_or_fail $? "Unexpected output."
}


function TC_tiffdither()            
{
	tc_register "tiffdither"
	tiffdither $LTPBIN/tiff/x.tif $TCTMP/dither.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	grep -q "dithered B&W version of x.tif" $TCTMP/dither.tif &&
	grep -q "min-is-black" $TCTMP/dither.tif
	tc_pass_or_fail $? "Unexpected output." || return

	tc_register "tiffdither -c"
	let TC_TOTAL+=1
	tiffdither -c zip $LTPBIN/tiff/x.tif $TCTMP/dither2.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	grep -q "dithered B&W version of x.tif" $TCTMP/dither2.tif &&
	grep -q "Deflate" $TCTMP/dither2.tif &&
	grep -q "min-is-black" $TCTMP/dither2.tif
	tc_pass_or_fail $? "Unexpected output."

	tc_register "tiffdither"
	let TC_TOTAL+=1
	tiffdither -f lsb2msb $LTPBIN/tiff/x.tif $TCTMP/dither3.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	grep -q "dithered B&W version of x.tif" $TCTMP/dither3.tif &&
	grep -q "lsb-to-msb" $TCTMP/dither3.tif 
	tc_pass_or_fail $? "Unexpected output."
}

function TC_tiffmedian()            
{
	tc_register "tiffmedian"
	tiffmedian $LTPBIN/tiff/mcp.tif $TCTMP/med.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	tiffinfo $TCTMP/med.tif | grep -q palette 
	tc_pass_or_fail $? "Unexpected output."

	tc_register "tiffmedian -fc"
	let TC_TOTAL+=1
	tiffmedian -f -c packbits $LTPBIN/tiff/mcp.tif $TCTMP/med2.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code" || return

	tiffinfo $TCTMP/med2.tif | grep -q PackBits 
	tc_pass_or_fail $? "Unexpected output."
}


function TC_tiffset()            
{
	Notest tiffset
}

function TC_tiffsplit()            
{
	tc_register "tiffsplit"
	tiffsplit  $TCTMP/z.tif >$stdout 2>$stderr
	tc_fail_if_bad $? "Incorrect return code." || return

	[ -e xaa.tif -a -e xab.tif ]
	tc_pass_or_fail $? "Unexpected output."
}


################################################################################
# main
################################################################################
tc_setup

# Check if supporting utilities are available
tc_exec_or_break  $REQUIRED || exit

FRC=0
for cmd in $COMMANDS
do
	TC_$cmd || FRC=$?
done
exit $FRC
