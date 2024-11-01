# Check the {fsname} filesystem
{fsname}_gf02 growfiles -W {fsname}_gf02 -d {tempdir}/{fsname} -b -e 1 -L 10 -B 805306368 -i 100 -I p -S 2 -u -f gf03_
{fsname}_gf03 growfiles -W {fsname}_gf03 -d {tempdir}/{fsname} -b -e 1 -g 1 -i 1 -S 150 -u -f gf05_
{fsname}_gf04 growfiles -W {fsname}_gf04 -d {tempdir}/{fsname} -b -e 1 -g 4090 -i 500 -t 39000 -u -f gf06_
{fsname}_gf05 growfiles -W {fsname}_gf05 -d {tempdir}/{fsname} -b -e 1 -g 5000 -i 500 -t 49900 -T10 -c9 -I p -u -f gf07_
{fsname}_gf16 growfiles -W {fsname}_gf16 -d {tempdir}/{fsname} -b -e 1 -i 0 -L 120 -B 805306368 -u -g 4090 -T 100 -t 408990 -l -C 10 -c 1000 -S 10 -f Lgf02_
{fsname}_gf17 growfiles -W {fsname}_gf17 -d {tempdir}/{fsname} -b -e 1 -i 0 -L 120 -B 805306368 -u -g 5000 -T 100 -t 499990 -l -C 10 -c 1000 -S 10 -f Lgf03_
{fsname}_gf18 growfiles -W {fsname}_gf18 -d {tempdir}/{fsname} -b -e 1 -i 0 -L 120 -B 805306368 -w -u -r 10-5000 -I r -T 10 -l -S 2 -f Lgf04_
{fsname}_gf19 growfiles -W {fsname}_gf19 -d {tempdir}/{fsname} -b -e 1 -g 5000 -i 500 -t 49900 -T10 -c9 -I p -o O_RDWR,O_CREAT,O_TRUNC -u -f gf08i_
{fsname}_gf12 mkfifo {tempdir}/{fsname}/gffifo17; growfiles -W {fsname}_gf12 -b -e 1 -u -i 0 -L 30 -B 805306368 {tempdir}/{fsname}/gffifo17
{fsname}_gf13 mkfifo {tempdir}/{fsname}/gffifo18; growfiles -W {fsname}_gf13 -b -e 1 -u -i 0 -L 30 -B 805306368 -I r -r 1-4096 {tempdir}/{fsname}/gffifo18
{fsname}_gf01 growfiles -W {fsname}_gf01 -b -e 1 -u -i 0 -L 20 -B 805306368 -w -C 1 -l -I r -T 10 {tempdir}/{fsname}/glseek20 {tempdir}/{fsname}/glseek20.2
{fsname}_gf06 growfiles -W {fsname}_gf06 -b -e 1 -u -r 1-5000 -R 0--1 -i 0 -L 30 -B 805306368 -C 1 {tempdir}/{fsname}/g_rand10 {tempdir}/{fsname}/g_rand10.2
{fsname}_gf07 growfiles -W {fsname}_gf07 -b -e 1 -u -r 1-5000 -R 0--2 -i 0 -L 30 -B 805306368 -C 1 -I p {tempdir}/{fsname}/g_rand13 {tempdir}/{fsname}/g_rand13.2
{fsname}_gf08 growfiles -W {fsname}_gf08 -b -e 1 -u -r 1-5000 -R 0--2 -i 0 -L 30 -B 805306368 -C 1 {tempdir}/{fsname}/g_rand11 {tempdir}/{fsname}/g_rand11.2
{fsname}_gf09 growfiles -W {fsname}_gf09 -b -e 1 -u -r 1-5000 -R 0--1 -i 0 -L 30 -B 805306368 -C 1 -I p {tempdir}/{fsname}/g_rand12 {tempdir}/{fsname}/g_rand12.2
{fsname}_gf10 growfiles -W {fsname}_gf10 -b -e 1 -u -r 1-5000 -i 0 -L 30 -B 805306368 -C 1 -I l {tempdir}/{fsname}/g_lio14 {tempdir}/{fsname}/g_lio14.2
{fsname}_gf11 growfiles -W {fsname}_gf11 -b -e 1 -u -r 1-5000 -i 0 -L 30 -B 805306368 -C 1 -I L {tempdir}/{fsname}/g_lio15 {tempdir}/{fsname}/g_lio15.2
{fsname}_gf14 growfiles -W {fsname}_gf14 -b -e 1 -u -i 0 -L 20 -B 805306368 -w -l -C 1 -T 10 {tempdir}/{fsname}/glseek19 {tempdir}/{fsname}/glseek19.2
{fsname}_gf15 growfiles -W {fsname}_gf15 -b -e 1 -u -r 1-49600 -I r -u -i 0 -L 120 -B 805306368 {tempdir}/{fsname}/Lgfile1
{fsname}_gf20 growfiles -W {fsname}_gf20 -D 0 -b -i 0 -L 60 -u -B 1000b -e 1 -r 1-256000:512 -R 512-256000 -T 4 {tempdir}/{fsname}/gfbigio-$$
{fsname}_gf21 growfiles -W {fsname}_gf21 -D 0 -b -i 0 -L 60 -u -B 1000b -e 1 -g 20480 -T 10 -t 20480 {tempdir}/{fsname}/gf-bld-$$
{fsname}_gf22 growfiles -W {fsname}_gf22 -D 0 -b -i 0 -L 60 -u -B 1000b -e 1 -g 20480 -T 10 -t 20480 {tempdir}/{fsname}/gf-bldf-$$
{fsname}_gf23 growfiles -W {fsname}_gf23 -D 0 -b -i 0 -L 60 -u -B 1000b -e 1 -r 512-64000:1024 -R 1-384000 -T 4 {tempdir}/{fsname}/gf-inf-$$
{fsname}_gf24 growfiles -W {fsname}_gf24 -D 0 -b -i 0 -L 60 -u -B 1000b -e 1 -g 20480 {tempdir}/{fsname}/gf-jbld-$$
{fsname}_gf25 growfiles -W {fsname}_gf25 -D 0 -b -i 0 -L 60 -u -B 1000b -e 1 -r 1024000-2048000:2048 -R 4095-2048000 -T 1 {tempdir}/{fsname}/gf-large-gs-$$
{fsname}_gf26 growfiles -W {fsname}_gf26 -D 0 -b -i 0 -L 60 -u -B 1000b -e 1 -r 128-32768:128 -R 512-64000 -T 4 {tempdir}/{fsname}/gfsmallio-$$
{fsname}_gf27 growfiles -W {fsname}_gf27 -b -D 0 -w -g 8b -C 1 -b -i 1000 -u {tempdir}/{fsname}/gfsparse-1-$$
{fsname}_gf28 growfiles -W {fsname}_gf28 -b -D 0 -w -g 16b -C 1 -b -i 1000 -u {tempdir}/{fsname}/gfsparse-2-$$
{fsname}_gf29 growfiles -W {fsname}_gf29 -b -D 0 -r 1-4096 -R 0-33554432 -i 0 -L 60 -B 805306368 -C 1 -u {tempdir}/{fsname}/gfsparse-3-$$
{fsname}_gf30 growfiles -W {fsname}_gf30 -D 0 -b -i 0 -L 60 -u -B 1000b -e 1 -o O_RDWR,O_CREAT,O_SYNC -g 20480 -T 10 -t 20480 {tempdir}/{fsname}/gf-sync-$$
{fsname}_plough01 fsplough -d {tempdir}/{fsname}
{fsname}_plough02 fsplough -R -d {tempdir}/{fsname}
{fsname}_plough03 fsplough -W -d {tempdir}/{fsname}
{fsname}_plough04 fsplough -RW -d {tempdir}/{fsname}
{fsname}_rwtest01 rwtest -N {fsname}_rwtest01 -c -q -i 60s  -f sync 10%25000:rw-sync-$$ 500b:{tempdir}/{fsname}/rwtest01%f
{fsname}_rwtest02 rwtest -N {fsname}_rwtest02 -c -q -i 60s  -f buffered 10%25000:rw-buffered-$$ 500b:{tempdir}/{fsname}/rwtest02%f
{fsname}_rwtest03 rwtest -N {fsname}_rwtest03 -c -q -i 60s -n 2  -f buffered -s mmread,mmwrite -m random -Dv 10%25000:mm-buff-$$ 500b:{tempdir}/{fsname}/rwtest03%f
{fsname}_rwtest04 rwtest -N {fsname}_rwtest04 -c -q -i 60s -n 2  -f sync -s mmread,mmwrite -m random -Dv 10%25000:mm-sync-$$ 500b:{tempdir}/{fsname}/rwtest04%f
{fsname}_rwtest05 rwtest -N {fsname}_rwtest05 -c -q -i 50 -T 64b 500b:{tempdir}/{fsname}/rwtest05%f
