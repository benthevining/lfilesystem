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

#include <string>
#include <string_view>
#include <memory>
#include <vector>

namespace limes::files::cli
{

class Mode
{
public:
	virtual ~Mode() = default;

	[[nodiscard]] virtual std::string_view getName() const = 0;

	virtual void outputHelp() const = 0;

	[[nodiscard]] virtual bool execute (int argc, char** argv) const = 0;

private:
};

using Modes = std::vector<std::unique_ptr<Mode>>;

[[nodiscard]] const Mode* getMode (std::string_view mode);

[[nodiscard]] const Modes& getAllModes();

}  // namespace limes::files::cli
