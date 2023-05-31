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

	[[nodiscard]] virtual std::string_view getName() = 0;

	[[nodiscard]] virtual std::string getHelpString() = 0;

	[[nodiscard]] virtual bool execute (int argc, char** argv) = 0;

private:
};

[[nodiscard]] std::unique_ptr<Mode> getMode (std::string_view mode);

[[nodiscard]] std::vector<std::unique_ptr<Mode>> getAllModes();

} // namespace limes::files::cli
