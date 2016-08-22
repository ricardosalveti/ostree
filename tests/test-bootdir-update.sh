#!/bin/bash

set -euo pipefail

. $(dirname $0)/libtest.sh

echo "1..2"

setup_os_repository "archive-z2" "uboot"

cd ${test_tmpdir}

mkdir -p osdata/usr/lib/ostree-boot
echo "1" > osdata/usr/lib/ostree-boot/1
mkdir -p osdata/usr/lib/ostree-boot/subdir
echo "2" > osdata/usr/lib/ostree-boot/subdir/2

${CMD_PREFIX} ostree --repo=testos-repo commit --tree=dir=osdata/ -b testos/buildmain/x86_64-runtime

${CMD_PREFIX} ostree --repo=sysroot/ostree/repo remote add --set=gpg-verify=false testos $(cat httpd-address)/ostree/testos-repo
${CMD_PREFIX} ostree --repo=sysroot/ostree/repo pull testos testos/buildmain/x86_64-runtime
${CMD_PREFIX} ostree admin deploy --karg=root=LABEL=rootfs --os=testos testos:testos/buildmain/x86_64-runtime

assert_has_file sysroot/boot/ostree/testos-${bootcsum}/vmlinuz-3.6.0
assert_not_has_file sysroot/boot/ostree/testos-${bootcsum}/1

echo "ok boot dir without .ostree-bootcsumdir-source"

touch osdata/usr/lib/ostree-boot/.ostree-bootcsumdir-source
${CMD_PREFIX} ostree --repo=testos-repo commit --tree=dir=osdata/ -b testos/buildmain/x86_64-runtime
${CMD_PREFIX} ostree admin upgrade --os=testos

assert_has_file sysroot/boot/ostree/testos-${bootcsum}/vmlinuz-3.6.0
assert_has_file sysroot/boot/ostree/testos-${bootcsum}/1
assert_file_has_content sysroot/boot/ostree/testos-${bootcsum}/1 "1"
assert_has_file sysroot/boot/ostree/testos-${bootcsum}/subdir/2
assert_file_has_content sysroot/boot/ostree/testos-${bootcsum}/subdir/2 "2"

echo "ok boot dir with .ostree-bootcsumdir-source"
