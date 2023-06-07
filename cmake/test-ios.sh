#!/bin/bash

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

# usage: test-ios.sh <device-id> <foo.app> [<args...>]

# note that the .app needs to be built with the simulator SDK, not the device SDK

set -euxo pipefail

DEVICE_ID="$1"
readonly DEVICE_ID

APP_PATH="$2"
readonly APP_PATH

shift 2

if [ ! -f "$APP_PATH" ]; then
	echo App does not exist at path '$APP_PATH'!
	exit 1
fi

APP_BUNDLE_ID=$(mdls -name kMDItemCFBundleIdentifier -r "$APP_PATH")
readonly APP_BUNDLE_ID

# TODO: ideally check if this device is already booted...
xcrun simctl boot "$DEVICE_ID"

xcrun simctl install "$DEVICE_ID" "$APP_PATH"

xcrun simctl launch --console-pty --terminate-running-process "$DEVICE_ID" "$APP_BUNDLE_ID" "$@"

readonly success=$?

# clean up

xcrun simctl shutdown "$DEVICE_ID"
xcrun simctl erase "$DEVICE_ID"

exit $success
