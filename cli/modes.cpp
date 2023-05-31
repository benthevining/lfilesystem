/*
 * ======================================================================================
 *  __    ____  __  __  ____  ___
 * (  )  (_  _)(  \/  )( ___)/ __)
 *  )(__  _)(_  )    (  )__) \__ \
 * (____)(____)(_/\/\_)(____)(___/
 *
 *  This file is part of the Limes open source library and is licensed under the terms of the GNU Public License.
 *
 *  Commercial licenses are available; contact the maintainers at ben.the.vining@gmail.com to inquire for details.
 *
 * ======================================================================================
 */

#include "./modes.h"

/*
basename
cat
cd (cd + run command)
chgrp
chmod
chown
cksum
cmp
compress
cp
dd
df
dirname
du
file
find
link
ln
ls
mkdir
mkfifo
mv
pathchk
pwd
readlink
rm
rmdir
split
touch
ulimit
uncompress
unlink
 */

namespace limes::files::cli
{

std::unique_ptr<Mode> getMode (std::string_view mode)
{
}

std::vector<std::unique_ptr<Mode>> getAllModes()
{
}

}  // namespace limes::files::cli
