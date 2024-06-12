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

#include "lfilesystem/lfilesystem_Export.h"
#include "lfilesystem/lfilesystem_SpecialDirectories.h"
#include "lfilesystem/lfilesystem_Directory.h"

// Android implementations are currently TODO!

namespace limes::files::dirs
{

Directory apps()
{
	// TODO: should this be /data/data or /data/app ?
	return Directory { "/system/app" };
}

static inline Directory getAndroidAppDataDir()
{
	// if (auto* env = getEnv())
	// {
	// 	LocalRef<jobject> applicationInfo (env->CallObjectMethod (getAppContext().get(), AndroidContext.getApplicationInfo));
	// 	LocalRef<jobject> jString (env->GetObjectField (applicationInfo.get(), AndroidApplicationInfo.publicSourceDir));

	// 	return { juceString ((jstring) jString.get()) };
	// }

	return {};
}

static inline Directory getAndroidDocumentsDir()
{
	// if (getAndroidSDKVersion() >= 19)
	// 	return getWellKnownFolder ("DIRECTORY_DOCUMENTS");

	// if (auto* env = getEnv())
	// 	return resolveAndroidDir (LocalRef<jobject> (env->CallStaticObjectMethod (AndroidEnvironment, AndroidEnvironment.getDataDirectory)));

	return {};
}

[[nodiscard]] LFILE_EXPORT Directory android_home()
{
	return getAndroidAppDataDir();
}

Directory commonAppData()
{
	return getAndroidAppDataDir();
}

Directory commonDocuments()
{
	return getAndroidDocumentsDir();
}

Directory desktop()
{
	return getAndroidAppDataDir();
}

Directory userDocuments()
{
	return getAndroidDocumentsDir();
}

Directory userAppData()
{
	return getAndroidAppDataDir();
}

Directory downloads()
{
	return {};
}

}  // namespace limes::files::dirs
