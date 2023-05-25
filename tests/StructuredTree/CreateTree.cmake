# ======================================================================================
#  __    ____  __  __  ____  ___
# (  )  (_  _)(  \/  )( ___)/ __)
#  )(__  _)(_  )    (  )__) \__ \
# (____)(____)(_/\/\_)(____)(___/
#
#  This file is part of the Limes open source library and is licensed under the terms of the GNU Public License.
#
#  Commercial licenses are available; contact the maintainers at ben.the.vining@gmail.com to inquire for details.
#
# ======================================================================================

if (NOT DEFINED TREE_ROOT)
	message (FATAL_ERROR "TREE_ROOT must be defined!")
endif ()

file (REMOVE_RECURSE "${TREE_ROOT}")

file (MAKE_DIRECTORY "${TREE_ROOT}")

foreach (filename IN ITEMS example.txt sample.omg .trial)
	file (TOUCH "${TREE_ROOT}/${filename}")
endforeach ()

set (subdirs Foo Bar Baz)

set (Foo_children hello.txt world.png)
set (Bar_children foo Bar.cmake)
set (Baz_children "")

foreach (subdirName IN LISTS subdirs)

	set (subdirPath "${TREE_ROOT}/${subdirName}")

	file (MAKE_DIRECTORY "${subdirPath}")

	foreach (filename IN LISTS ${subdirName}_children)
		file (TOUCH "${subdirPath}/${filename}")
	endforeach ()

endforeach ()

file (WRITE "${TREE_ROOT}/Foo/hello.txt" "The quick brown fox jumps over the lazy dog.")
