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
#include <algorithm>
#include "./modes/cat.h"
#include "./modes/basename.h"
#include "./modes/cp.h"
#include "./modes/df.h"

/*
cd (cd + run command)
chgrp
chmod
chown
cksum
cmp
compress
dd
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

const Mode* getMode (std::string_view mode)
{
	const auto& allModes = getAllModes();

	const auto result = std::find_if (std::begin (allModes),
								std::end (allModes),
								[mode](const auto& m)
								{
		return m->getName() == mode;
	});

	if (result == std::end (allModes))
		return nullptr;

	return result->get();
}

struct AllModes final
{
	Modes modes;

	AllModes()
	{
		modes.reserve (4);

		modes.emplace_back (std::make_unique<modes::Cat>());
		modes.emplace_back (std::make_unique<modes::Basename>());
		modes.emplace_back (std::make_unique<modes::CP>());
		modes.emplace_back (std::make_unique<modes::DF>());
	}
};

const Modes& getAllModes()
{
	static AllModes holder;

	return holder.modes;
}

}  // namespace limes::files::cli
