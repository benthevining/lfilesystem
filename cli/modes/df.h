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

#pragma once

#include "../modes.h"
#include <lfilesystem/lfilesystem.h>
#include <string_view>
#include <iostream>
#include <vector>

namespace limes::files::cli::modes
{

class DF final : public Mode
{
public:
	[[nodiscard]] std::string_view getName() const final
	{
		return "df";
	}

	void outputHelp() const final
	{

	}

	[[nodiscard]] bool execute (int argc, char** argv) const final
	{

	}
};

}
